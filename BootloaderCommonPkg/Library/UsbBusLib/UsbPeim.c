/** @file
The module to produce Usb Bus PPI.

Copyright (c) 2006 - 2017, Intel Corporation. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UsbPeim.h"
#include "HubPeim.h"
#include "PeiUsbLib.h"

USB_IO_CALLBACK        mUsbIoCb;

//
// UsbIo PPI interface function
//
PEI_USB_IO_PPI         mUsbIoPpi = {
  PeiUsbControlTransfer,
  PeiUsbBulkTransfer,
  PeiUsbGetInterfaceDescriptor,
  PeiUsbGetEndpointDescriptor,
  PeiUsbPortReset
};

EFI_PEI_PPI_DESCRIPTOR mUsbIoPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  NULL,
  NULL
};

/**
  The enumeration routine to detect device change.

  @param  PeiServices            Describes the list of possible PEI Services.
  @param  UsbHcPpi               The pointer of PEI_USB_HOST_CONTROLLER_PPI instance.
  @param  Usb2HcPpi              The pointer of PEI_USB2_HOST_CONTROLLER_PPI instance.

  @retval EFI_SUCCESS            The usb is enumerated successfully.
  @retval EFI_OUT_OF_RESOURCES   Can't allocate memory resource.
  @retval Others                 Other failure occurs.

**/
EFI_STATUS
PeiUsbEnumeration (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN PEI_USB_HOST_CONTROLLER_PPI    *UsbHcPpi,
  IN PEI_USB2_HOST_CONTROLLER_PPI    *Usb2HcPpi
  );

/**
  Configure new detected usb device.

  @param  PeiServices            Describes the list of possible PEI Services.
  @param  PeiUsbDevice           The pointer of PEI_USB_DEVICE instance.
  @param  Port                   The port to be configured.
  @param  DeviceAddress          The device address to be configured.

  @retval EFI_SUCCESS            The new detected usb device is configured successfully.
  @retval EFI_OUT_OF_RESOURCES   Can't allocate memory resource.
  @retval Others                 Other failure occurs.

**/
EFI_STATUS
PeiConfigureUsbDevice (
  IN     EFI_PEI_SERVICES    **PeiServices,
  IN     PEI_USB_DEVICE      *PeiUsbDevice,
  IN     UINT8               Port,
  IN OUT UINT8               *DeviceAddress
  );

/**
  Get all configurations from a detected usb device.

  @param  PeiServices            Describes the list of possible PEI Services.
  @param  PeiUsbDevice           The pointer of PEI_USB_DEVICE instance.

  @retval EFI_SUCCESS            The new detected usb device is configured successfully.
  @retval EFI_OUT_OF_RESOURCES   Can't allocate memory resource.
  @retval Others                 Other failure occurs.

**/
EFI_STATUS
PeiUsbGetAllConfiguration (
  IN EFI_PEI_SERVICES   **PeiServices,
  IN PEI_USB_DEVICE     *PeiUsbDevice
  );

/**
  Get the start position of next wanted descriptor.

  @param  Buffer            Buffer containing data to parse.
  @param  Length            Buffer length.
  @param  DescType          Descriptor type.
  @param  DescLength        Descriptor length.
  @param  ParsedBytes       Bytes has been parsed.

  @retval EFI_SUCCESS       Get wanted descriptor successfully.
  @retval EFI_DEVICE_ERROR  Error occurred.

**/
EFI_STATUS
GetExpectedDescriptor (
  IN  UINT8       *Buffer,
  IN  UINTN       Length,
  IN  UINT8       DescType,
  IN  UINT8       DescLength,
  OUT UINTN       *ParsedBytes
  );

/**
  The Hub Enumeration just scans the hub ports one time. It also
  doesn't support hot-plug.

  @param  PeiServices            Describes the list of possible PEI Services.
  @param  PeiUsbDevice           The pointer of PEI_USB_DEVICE instance.
  @param  CurrentAddress         The DeviceAddress of usb device.

  @retval EFI_SUCCESS            The usb hub is enumerated successfully.
  @retval EFI_OUT_OF_RESOURCES   Can't allocate memory resource.
  @retval Others                 Other failure occurs.

**/
EFI_STATUS
PeiHubEnumeration (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN PEI_USB_DEVICE                 *PeiUsbDevice,
  IN UINT8                          *CurrentAddress
  )
{
  UINTN                 Index;
  EFI_STATUS            Status;
  PEI_USB_IO_PPI        *UsbIoPpi;
  EFI_USB_PORT_STATUS   PortStatus;
  UINTN                 MemPages;
  EFI_PHYSICAL_ADDRESS  AllocateAddress;
  PEI_USB_DEVICE        *NewPeiUsbDevice;
  UINTN                 InterfaceIndex;
  UINTN                 EndpointIndex;


  UsbIoPpi    = &PeiUsbDevice->UsbIoPpi;

  DEBUG ((DEBUG_VERBOSE, "PeiHubEnumeration: DownStreamPortNo: %x\n", PeiUsbDevice->DownStreamPortNo));

  for (Index = 0; Index < PeiUsbDevice->DownStreamPortNo; Index++) {

    Status = PeiHubGetPortStatus (
               PeiServices,
               UsbIoPpi,
               (UINT8) (Index + 1),
               (UINT32 *) &PortStatus
               );

    if (EFI_ERROR (Status)) {
      continue;
    }

    DEBUG ((DEBUG_VERBOSE, "USB Status --- Port: %x ConnectChange[%04x] Status[%04x]\n", Index, PortStatus.PortChangeStatus,
            PortStatus.PortStatus));
    //
    // Only handle connection/enable/overcurrent/reset change.
    //
    if ((PortStatus.PortChangeStatus & (USB_PORT_STAT_C_CONNECTION | USB_PORT_STAT_C_ENABLE | USB_PORT_STAT_C_OVERCURRENT |
                                        USB_PORT_STAT_C_RESET)) == 0) {
      continue;
    } else {
      if (IsPortConnect (PortStatus.PortStatus)) {
        //
        // Begin to deal with the new device
        //
        MemPages = sizeof (PEI_USB_DEVICE) / EFI_PAGE_SIZE + 1;
        Status = PeiServicesAllocatePages (
                   EfiBootServicesCode,
                   MemPages,
                   &AllocateAddress
                   );
        if (EFI_ERROR (Status)) {
          return EFI_OUT_OF_RESOURCES;
        }

        NewPeiUsbDevice = (PEI_USB_DEVICE *) ((UINTN) AllocateAddress);
        ZeroMem (NewPeiUsbDevice, sizeof (PEI_USB_DEVICE));

        NewPeiUsbDevice->Signature        = PEI_USB_DEVICE_SIGNATURE;
        NewPeiUsbDevice->DeviceAddress    = 0;
        NewPeiUsbDevice->MaxPacketSize0   = 8;
        NewPeiUsbDevice->DataToggle       = 0;
        CopyMem (
          & (NewPeiUsbDevice->UsbIoPpi),
          &mUsbIoPpi,
          sizeof (PEI_USB_IO_PPI)
          );
        CopyMem (
          & (NewPeiUsbDevice->UsbIoPpiList),
          &mUsbIoPpiList,
          sizeof (EFI_PEI_PPI_DESCRIPTOR)
          );
        NewPeiUsbDevice->UsbIoPpiList.Ppi = &NewPeiUsbDevice->UsbIoPpi;
        NewPeiUsbDevice->AllocateAddress  = (UINTN) AllocateAddress;
        NewPeiUsbDevice->UsbHcPpi         = PeiUsbDevice->UsbHcPpi;
        NewPeiUsbDevice->Usb2HcPpi        = PeiUsbDevice->Usb2HcPpi;
        NewPeiUsbDevice->Tier             = (UINT8) (PeiUsbDevice->Tier + 1);
        NewPeiUsbDevice->IsHub            = 0x0;
        NewPeiUsbDevice->DownStreamPortNo = 0x0;
        NewPeiUsbDevice->Port             = (UINT8)Index;
        NewPeiUsbDevice->Parent           = &PeiUsbDevice->UsbIoPpi;

        if (((PortStatus.PortChangeStatus & USB_PORT_STAT_C_RESET) == 0) ||
            ((PortStatus.PortStatus & (USB_PORT_STAT_CONNECTION | USB_PORT_STAT_ENABLE)) == 0)) {
          //
          // If the port already has reset change flag and is connected and enabled, skip the port reset logic.
          //
          PeiResetHubPort (PeiServices, UsbIoPpi, (UINT8) (Index + 1));

          PeiHubGetPortStatus (
            PeiServices,
            UsbIoPpi,
            (UINT8) (Index + 1),
            (UINT32 *) &PortStatus
            );
        } else {
          PeiHubClearPortFeature (
            PeiServices,
            UsbIoPpi,
            (UINT8) (Index + 1),
            EfiUsbPortResetChange
            );
        }

        NewPeiUsbDevice->DeviceSpeed = (UINT8) PeiUsbGetDeviceSpeed (PortStatus.PortStatus);
        DEBUG ((DEBUG_VERBOSE, "Device Speed =%d\n", PeiUsbDevice->DeviceSpeed));

        if (USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_SUPER_SPEED)) {
          NewPeiUsbDevice->MaxPacketSize0 = 512;
        } else if (USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_HIGH_SPEED)) {
          NewPeiUsbDevice->MaxPacketSize0 = 64;
        } else if (USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_LOW_SPEED)) {
          NewPeiUsbDevice->MaxPacketSize0 = 8;
        } else {
          NewPeiUsbDevice->MaxPacketSize0 = 8;
        }

        if (NewPeiUsbDevice->DeviceSpeed != EFI_USB_SPEED_HIGH) {
          if (PeiUsbDevice->DeviceSpeed == EFI_USB_SPEED_HIGH) {
            NewPeiUsbDevice->Translator.TranslatorPortNumber = (UINT8)Index;
            NewPeiUsbDevice->Translator.TranslatorHubAddress = *CurrentAddress;
          } else {
            CopyMem (& (NewPeiUsbDevice->Translator), & (PeiUsbDevice->Translator), sizeof (EFI_USB2_HC_TRANSACTION_TRANSLATOR));
          }
        }

        //
        // Configure that Usb Device
        //
        Status = PeiConfigureUsbDevice (
                   PeiServices,
                   NewPeiUsbDevice,
                   (UINT8) (Index + 1),
                   CurrentAddress
                   );

        if (EFI_ERROR (Status)) {
          continue;
        }
        DEBUG ((DEBUG_VERBOSE, "PeiHubEnumeration: PeiConfigureUsbDevice Success\n"));

        Status = PeiServicesInstallPpi (&NewPeiUsbDevice->UsbIoPpiList);

        if (NewPeiUsbDevice->InterfaceDesc->InterfaceClass == 0x09) {
          NewPeiUsbDevice->IsHub  = 0x1;

          Status = PeiDoHubConfig (PeiServices, NewPeiUsbDevice);
          if (EFI_ERROR (Status)) {
            return Status;
          }

          PeiHubEnumeration (PeiServices, NewPeiUsbDevice, CurrentAddress);
        }

        for (InterfaceIndex = 1; InterfaceIndex < NewPeiUsbDevice->ConfigDesc->NumInterfaces; InterfaceIndex++) {
          //
          // Begin to deal with the new device
          //
          MemPages = sizeof (PEI_USB_DEVICE) / EFI_PAGE_SIZE + 1;
          Status = PeiServicesAllocatePages (
                     EfiBootServicesCode,
                     MemPages,
                     &AllocateAddress
                     );
          if (EFI_ERROR (Status)) {
            return EFI_OUT_OF_RESOURCES;
          }
          CopyMem ((VOID *) (UINTN)AllocateAddress, NewPeiUsbDevice, sizeof (PEI_USB_DEVICE));
          NewPeiUsbDevice = (PEI_USB_DEVICE *) ((UINTN) AllocateAddress);
          NewPeiUsbDevice->AllocateAddress  = (UINTN) AllocateAddress;
          NewPeiUsbDevice->UsbIoPpiList.Ppi = &NewPeiUsbDevice->UsbIoPpi;
          NewPeiUsbDevice->InterfaceDesc = NewPeiUsbDevice->InterfaceDescList[InterfaceIndex];
          for (EndpointIndex = 0; EndpointIndex < NewPeiUsbDevice->InterfaceDesc->NumEndpoints; EndpointIndex++) {
            NewPeiUsbDevice->EndpointDesc[EndpointIndex] = NewPeiUsbDevice->EndpointDescList[InterfaceIndex][EndpointIndex];
          }

          Status = PeiServicesInstallPpi (&NewPeiUsbDevice->UsbIoPpiList);

          if (NewPeiUsbDevice->InterfaceDesc->InterfaceClass == 0x09) {
            NewPeiUsbDevice->IsHub  = 0x1;

            Status = PeiDoHubConfig (PeiServices, NewPeiUsbDevice);
            if (EFI_ERROR (Status)) {
              return Status;
            }

            PeiHubEnumeration (PeiServices, NewPeiUsbDevice, CurrentAddress);
          }
        }
      }
    }
  }


  return EFI_SUCCESS;
}

/**
  The enumeration routine to detect device change.

  @param  PeiServices            Describes the list of possible PEI Services.
  @param  UsbHcPpi               The pointer of PEI_USB_HOST_CONTROLLER_PPI instance.
  @param  Usb2HcPpi              The pointer of PEI_USB2_HOST_CONTROLLER_PPI instance.

  @retval EFI_SUCCESS            The usb is enumerated successfully.
  @retval EFI_OUT_OF_RESOURCES   Can't allocate memory resource.
  @retval Others                 Other failure occurs.

**/
EFI_STATUS
PeiUsbEnumeration (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN PEI_USB_HOST_CONTROLLER_PPI    *UsbHcPpi,
  IN PEI_USB2_HOST_CONTROLLER_PPI   *Usb2HcPpi
  )
{
  UINT8                 NumOfRootPort;
  EFI_STATUS            Status;
  UINT8                 Index;
  EFI_USB_PORT_STATUS   PortStatus;
  PEI_USB_DEVICE        *PeiUsbDevice;
  UINTN                 MemPages;
  EFI_PHYSICAL_ADDRESS  AllocateAddress;
  UINT8                 CurrentAddress;
  UINTN                 InterfaceIndex;
  UINTN                 EndpointIndex;

  CurrentAddress = 0;
  if (Usb2HcPpi != NULL) {
    Usb2HcPpi->GetRootHubPortNumber (
      PeiServices,
      Usb2HcPpi,
      (UINT8 *) &NumOfRootPort
      );
  } else if (UsbHcPpi != NULL) {
    UsbHcPpi->GetRootHubPortNumber (
      PeiServices,
      UsbHcPpi,
      (UINT8 *) &NumOfRootPort
      );
  } else {
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_VERBOSE, "PeiUsbEnumeration: NumOfRootPort: %x\n", NumOfRootPort));

  for (Index = 0; Index < NumOfRootPort; Index++) {
    //
    // First get root port status to detect changes happen
    //
    if (Usb2HcPpi != NULL) {
      Usb2HcPpi->GetRootHubPortStatus (
        PeiServices,
        Usb2HcPpi,
        (UINT8) Index,
        &PortStatus
        );
    } else {
      UsbHcPpi->GetRootHubPortStatus (
        PeiServices,
        UsbHcPpi,
        (UINT8) Index,
        &PortStatus
        );
    }
    DEBUG ((DEBUG_VERBOSE, "USB Status --- Port: %x ConnectChange[%04x] Status[%04x]\n", Index, PortStatus.PortChangeStatus,
            PortStatus.PortStatus));
    //
    // Only handle connection/enable/overcurrent/reset change.
    //
    if ((PortStatus.PortChangeStatus & (USB_PORT_STAT_C_CONNECTION | USB_PORT_STAT_C_ENABLE | USB_PORT_STAT_C_OVERCURRENT |
                                        USB_PORT_STAT_C_RESET)) == 0) {
      continue;
    } else {
      if (IsPortConnect (PortStatus.PortStatus)) {
        MemPages = sizeof (PEI_USB_DEVICE) / EFI_PAGE_SIZE + 1;
        Status = PeiServicesAllocatePages (
                   EfiBootServicesCode,
                   MemPages,
                   &AllocateAddress
                   );
        if (EFI_ERROR (Status)) {
          return EFI_OUT_OF_RESOURCES;
        }

        PeiUsbDevice = (PEI_USB_DEVICE *) ((UINTN) AllocateAddress);
        ZeroMem (PeiUsbDevice, sizeof (PEI_USB_DEVICE));

        PeiUsbDevice->Signature         = PEI_USB_DEVICE_SIGNATURE;
        PeiUsbDevice->DeviceAddress     = 0;
        PeiUsbDevice->MaxPacketSize0    = 8;
        PeiUsbDevice->DataToggle        = 0;
        CopyMem (
          & (PeiUsbDevice->UsbIoPpi),
          &mUsbIoPpi,
          sizeof (PEI_USB_IO_PPI)
          );
        CopyMem (
          & (PeiUsbDevice->UsbIoPpiList),
          &mUsbIoPpiList,
          sizeof (EFI_PEI_PPI_DESCRIPTOR)
          );
        PeiUsbDevice->UsbIoPpiList.Ppi  = &PeiUsbDevice->UsbIoPpi;
        PeiUsbDevice->AllocateAddress   = (UINTN) AllocateAddress;
        PeiUsbDevice->UsbHcPpi          = UsbHcPpi;
        PeiUsbDevice->Usb2HcPpi         = Usb2HcPpi;
        PeiUsbDevice->IsHub             = 0x0;
        PeiUsbDevice->DownStreamPortNo  = 0x0;
        PeiUsbDevice->Port              = Index;
        PeiUsbDevice->Parent            = NULL;

        if (((PortStatus.PortChangeStatus & USB_PORT_STAT_C_RESET) == 0) ||
            ((PortStatus.PortStatus & (USB_PORT_STAT_CONNECTION | USB_PORT_STAT_ENABLE)) == 0)) {
          //
          // If the port already has reset change flag and is connected and enabled, skip the port reset logic.
          //
          ResetRootPort (
            PeiServices,
            PeiUsbDevice->UsbHcPpi,
            PeiUsbDevice->Usb2HcPpi,
            Index,
            0
            );

          if (Usb2HcPpi != NULL) {
            Usb2HcPpi->GetRootHubPortStatus (
              PeiServices,
              Usb2HcPpi,
              (UINT8) Index,
              &PortStatus
              );
          } else {
            UsbHcPpi->GetRootHubPortStatus (
              PeiServices,
              UsbHcPpi,
              (UINT8) Index,
              &PortStatus
              );
          }
        } else {
          if (Usb2HcPpi != NULL) {
            Usb2HcPpi->ClearRootHubPortFeature (
              PeiServices,
              Usb2HcPpi,
              (UINT8) Index,
              EfiUsbPortResetChange
              );
          } else {
            UsbHcPpi->ClearRootHubPortFeature (
              PeiServices,
              UsbHcPpi,
              (UINT8) Index,
              EfiUsbPortResetChange
              );
          }
        }

        PeiUsbDevice->DeviceSpeed = (UINT8) PeiUsbGetDeviceSpeed (PortStatus.PortStatus);
        DEBUG ((DEBUG_VERBOSE, "Device Speed =%d\n", PeiUsbDevice->DeviceSpeed));

        if (USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_SUPER_SPEED)) {
          PeiUsbDevice->MaxPacketSize0 = 512;
        } else if (USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_HIGH_SPEED)) {
          PeiUsbDevice->MaxPacketSize0 = 64;
        } else if (USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_LOW_SPEED)) {
          PeiUsbDevice->MaxPacketSize0 = 8;
        } else {
          PeiUsbDevice->MaxPacketSize0 = 8;
        }

        //
        // Configure that Usb Device
        //
        Status = PeiConfigureUsbDevice (
                   PeiServices,
                   PeiUsbDevice,
                   Index,
                   &CurrentAddress
                   );

        if (EFI_ERROR (Status)) {
          continue;
        }
        DEBUG ((DEBUG_VERBOSE, "PeiUsbEnumeration: PeiConfigureUsbDevice Success\n"));

        Status = PeiServicesInstallPpi (&PeiUsbDevice->UsbIoPpiList);

        if (PeiUsbDevice->InterfaceDesc->InterfaceClass == 0x09) {
          PeiUsbDevice->IsHub = 0x1;

          Status = PeiDoHubConfig (PeiServices, PeiUsbDevice);
          if (EFI_ERROR (Status)) {
            return Status;
          }

          PeiHubEnumeration (PeiServices, PeiUsbDevice, &CurrentAddress);
        }

        for (InterfaceIndex = 1; InterfaceIndex < PeiUsbDevice->ConfigDesc->NumInterfaces; InterfaceIndex++) {
          //
          // Begin to deal with the new device
          //
          MemPages = sizeof (PEI_USB_DEVICE) / EFI_PAGE_SIZE + 1;
          Status = PeiServicesAllocatePages (
                     EfiBootServicesCode,
                     MemPages,
                     &AllocateAddress
                     );
          if (EFI_ERROR (Status)) {
            return EFI_OUT_OF_RESOURCES;
          }
          CopyMem ((VOID *) (UINTN)AllocateAddress, PeiUsbDevice, sizeof (PEI_USB_DEVICE));
          PeiUsbDevice = (PEI_USB_DEVICE *) ((UINTN) AllocateAddress);
          PeiUsbDevice->AllocateAddress  = (UINTN) AllocateAddress;
          PeiUsbDevice->UsbIoPpiList.Ppi = &PeiUsbDevice->UsbIoPpi;
          PeiUsbDevice->InterfaceDesc = PeiUsbDevice->InterfaceDescList[InterfaceIndex];
          for (EndpointIndex = 0; EndpointIndex < PeiUsbDevice->InterfaceDesc->NumEndpoints; EndpointIndex++) {
            PeiUsbDevice->EndpointDesc[EndpointIndex] = PeiUsbDevice->EndpointDescList[InterfaceIndex][EndpointIndex];
          }

          Status = PeiServicesInstallPpi (&PeiUsbDevice->UsbIoPpiList);

          if (PeiUsbDevice->InterfaceDesc->InterfaceClass == 0x09) {
            PeiUsbDevice->IsHub = 0x1;

            Status = PeiDoHubConfig (PeiServices, PeiUsbDevice);
            if (EFI_ERROR (Status)) {
              return Status;
            }

            PeiHubEnumeration (PeiServices, PeiUsbDevice, &CurrentAddress);
          }
        }
      } else {
        //
        // Disconnect change happen, currently we don't support
        //
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Configure new detected usb device.

  @param  PeiServices            Describes the list of possible PEI Services.
  @param  PeiUsbDevice           The pointer of PEI_USB_DEVICE instance.
  @param  Port                   The port to be configured.
  @param  DeviceAddress          The device address to be configured.

  @retval EFI_SUCCESS            The new detected usb device is configured successfully.
  @retval EFI_OUT_OF_RESOURCES   Can't allocate memory resource.
  @retval Others                 Other failure occurs.

**/
EFI_STATUS
PeiConfigureUsbDevice (
  IN EFI_PEI_SERVICES   **PeiServices,
  IN PEI_USB_DEVICE     *PeiUsbDevice,
  IN UINT8              Port,
  IN OUT UINT8          *DeviceAddress
  )
{
  EFI_USB_DEVICE_DESCRIPTOR   DeviceDescriptor;
  EFI_STATUS                  Status;
  PEI_USB_IO_PPI              *UsbIoPpi;
  UINT8                       Retry;
  UINT8                       StrLoop;
  UINT32                      StrOffset;
  UINT32                      WcharCnt;
  EFI_USB_STRING_DESCRIPTOR  *StringDescriptor;

  UsbIoPpi = &PeiUsbDevice->UsbIoPpi;
  Status   = EFI_SUCCESS;
  ZeroMem (&DeviceDescriptor, sizeof (EFI_USB_DEVICE_DESCRIPTOR));
  //
  // Get USB device descriptor
  //

  for (Retry = 0; Retry < 3; Retry ++) {
    Status = PeiUsbGetDescriptor (
               PeiServices,
               UsbIoPpi,
               (USB_DT_DEVICE << 8),
               0,
               8,
               &DeviceDescriptor
               );

    if (!EFI_ERROR (Status)) {
      DEBUG ((DEBUG_VERBOSE, "PeiUsbGet Device Descriptor the %d time Success\n", Retry));
      break;
    }
  }

  if (Retry == 3) {
    DEBUG ((DEBUG_ERROR, "PeiUsbGet Device Descriptor fail: %x %r\n", Retry, Status));
    return Status;
  }

  if ((DeviceDescriptor.BcdUSB >= 0x0300) && (DeviceDescriptor.MaxPacketSize0 == 9)) {
    PeiUsbDevice->MaxPacketSize0 = 1 << 9;
  } else {
    PeiUsbDevice->MaxPacketSize0 = DeviceDescriptor.MaxPacketSize0;
  }

  (*DeviceAddress) ++;

  Status = PeiUsbSetDeviceAddress (
             PeiServices,
             UsbIoPpi,
             *DeviceAddress
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PeiUsbSetDeviceAddress Failed: %r\n", Status));
    return Status;
  }
  MicroSecondDelay (USB_SET_DEVICE_ADDRESS_STALL);

  PeiUsbDevice->DeviceAddress = *DeviceAddress;

  //
  // Get whole USB device descriptor
  //
  Status = PeiUsbGetDescriptor (
             PeiServices,
             UsbIoPpi,
             (USB_DT_DEVICE << 8),
             0,
             (UINT16) sizeof (EFI_USB_DEVICE_DESCRIPTOR),
             &DeviceDescriptor
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PeiUsbGetDescriptor First Failed\n"));
    return Status;
  }

  //
  // Get manufacturer and product name string
  //
  PeiUsbDevice->ProductName[0] = 0;
  for (StrLoop = 0; StrLoop < 2; StrLoop++) {
    Status = PeiUsbGetDescriptor (
               PeiServices,
               UsbIoPpi,
               (USB_DT_STRING << 8) |
                 ((StrLoop == 0) ? DeviceDescriptor.StrManufacturer : DeviceDescriptor.StrProduct),
               0x0409,
               (UINT16) sizeof (PeiUsbDevice->ConfigurationData),
               PeiUsbDevice->ConfigurationData
               );
    if (!EFI_ERROR (Status)) {
      StringDescriptor = (EFI_USB_STRING_DESCRIPTOR *)PeiUsbDevice->ConfigurationData;
      StrOffset = OFFSET_OF(EFI_USB_STRING_DESCRIPTOR, String);
      if (StringDescriptor->Length > StrOffset) {
        WcharCnt = ARRAY_SIZE(PeiUsbDevice->ProductName) - 1;
        StrnCatS (PeiUsbDevice->ProductName, WcharCnt,
                  StringDescriptor->String,  (StringDescriptor->Length - StrOffset) / sizeof(CHAR16));
        StrCatS  (PeiUsbDevice->ProductName, WcharCnt, L" ");
      }
    }
  }

  //
  // Get its default configuration and its first interface
  //
  Status = PeiUsbGetAllConfiguration (
             PeiServices,
             PeiUsbDevice
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  MicroSecondDelay (USB_GET_CONFIG_DESCRIPTOR_STALL);

  Status = PeiUsbSetConfiguration (
             PeiServices,
             UsbIoPpi
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Get all configurations from a detected usb device.

  @param  PeiServices            Describes the list of possible PEI Services.
  @param  PeiUsbDevice           The pointer of PEI_USB_DEVICE instance.

  @retval EFI_SUCCESS            The new detected usb device is configured successfully.
  @retval EFI_OUT_OF_RESOURCES   Can't allocate memory resource.
  @retval Others                 Other failure occurs.

**/
EFI_STATUS
PeiUsbGetAllConfiguration (
  IN EFI_PEI_SERVICES   **PeiServices,
  IN PEI_USB_DEVICE     *PeiUsbDevice
  )
{
  EFI_STATUS                Status;
  EFI_USB_CONFIG_DESCRIPTOR *ConfigDesc;
  PEI_USB_IO_PPI            *UsbIoPpi;
  UINT16                    ConfigDescLength;
  UINT8                     *Ptr;
  UINTN                     SkipBytes;
  UINTN                     LengthLeft;
  UINTN                     InterfaceIndex;
  UINTN                     Index;
  UINTN                     NumOfEndpoint;

  UsbIoPpi = &PeiUsbDevice->UsbIoPpi;

  //
  // First get its 4-byte configuration descriptor
  //
  Status = PeiUsbGetDescriptor (
             PeiServices,
             UsbIoPpi,
             (USB_DT_CONFIG << 8), // Value
             0,      // Index
             4,      // Length
             PeiUsbDevice->ConfigurationData
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PeiUsbGet Config Descriptor First Failed\n"));
    return Status;
  }
  MicroSecondDelay (USB_GET_CONFIG_DESCRIPTOR_STALL);

  ConfigDesc        = (EFI_USB_CONFIG_DESCRIPTOR *) PeiUsbDevice->ConfigurationData;
  ConfigDescLength  = ConfigDesc->TotalLength;

  //
  // Then we get the total descriptors for this configuration
  //
  Status = PeiUsbGetDescriptor (
             PeiServices,
             UsbIoPpi,
             (USB_DT_CONFIG << 8),
             0,
             ConfigDescLength,
             PeiUsbDevice->ConfigurationData
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PeiUsbGet Config Descriptor all Failed\n"));
    return Status;
  }
  //
  // Parse this configuration descriptor
  // First get the current config descriptor;
  //
  Status = GetExpectedDescriptor (
             PeiUsbDevice->ConfigurationData,
             ConfigDescLength,
             USB_DT_CONFIG,
             (UINT8) sizeof (EFI_USB_CONFIG_DESCRIPTOR),
             &SkipBytes
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Ptr                       = PeiUsbDevice->ConfigurationData + SkipBytes;
  PeiUsbDevice->ConfigDesc  = (EFI_USB_CONFIG_DESCRIPTOR *) Ptr;

  Ptr += sizeof (EFI_USB_CONFIG_DESCRIPTOR);
  LengthLeft = ConfigDescLength - SkipBytes - sizeof (EFI_USB_CONFIG_DESCRIPTOR);

  for (InterfaceIndex = 0; InterfaceIndex < PeiUsbDevice->ConfigDesc->NumInterfaces; InterfaceIndex++) {

    //
    // Get the interface descriptor
    //
    Status = GetExpectedDescriptor (
               Ptr,
               LengthLeft,
               USB_DT_INTERFACE,
               (UINT8) sizeof (EFI_USB_INTERFACE_DESCRIPTOR),
               &SkipBytes
               );

    if (EFI_ERROR (Status)) {
      return Status;
    }

    Ptr += SkipBytes;
    if (InterfaceIndex == 0) {
      PeiUsbDevice->InterfaceDesc = (EFI_USB_INTERFACE_DESCRIPTOR *) Ptr;
    }
    PeiUsbDevice->InterfaceDescList[InterfaceIndex] = (EFI_USB_INTERFACE_DESCRIPTOR *) Ptr;

    Ptr += sizeof (EFI_USB_INTERFACE_DESCRIPTOR);
    LengthLeft -= SkipBytes;
    LengthLeft -= sizeof (EFI_USB_INTERFACE_DESCRIPTOR);

    //
    // Parse all the endpoint descriptor within this interface
    //
    NumOfEndpoint = PeiUsbDevice->InterfaceDescList[InterfaceIndex]->NumEndpoints;
    ASSERT (NumOfEndpoint <= MAX_ENDPOINT);

    for (Index = 0; Index < NumOfEndpoint; Index++) {
      //
      // Get the endpoint descriptor
      //
      Status = GetExpectedDescriptor (
                 Ptr,
                 LengthLeft,
                 USB_DT_ENDPOINT,
                 (UINT8) sizeof (EFI_USB_ENDPOINT_DESCRIPTOR),
                 &SkipBytes
                 );

      if (EFI_ERROR (Status)) {
        return Status;
      }

      Ptr += SkipBytes;
      if (InterfaceIndex == 0) {
        PeiUsbDevice->EndpointDesc[Index] = (EFI_USB_ENDPOINT_DESCRIPTOR *) Ptr;
      }
      PeiUsbDevice->EndpointDescList[InterfaceIndex][Index] = (EFI_USB_ENDPOINT_DESCRIPTOR *) Ptr;

      Ptr += sizeof (EFI_USB_ENDPOINT_DESCRIPTOR);
      LengthLeft -= SkipBytes;
      LengthLeft -= sizeof (EFI_USB_ENDPOINT_DESCRIPTOR);
    }
  }

  return EFI_SUCCESS;
}

/**
  Get the start position of next wanted descriptor.

  @param  Buffer            Buffer containing data to parse.
  @param  Length            Buffer length.
  @param  DescType          Descriptor type.
  @param  DescLength        Descriptor length.
  @param  ParsedBytes       Bytes has been parsed.

  @retval EFI_SUCCESS       Get wanted descriptor successfully.
  @retval EFI_DEVICE_ERROR  Error occurred.

**/
EFI_STATUS
GetExpectedDescriptor (
  IN  UINT8       *Buffer,
  IN  UINTN       Length,
  IN  UINT8       DescType,
  IN  UINT8       DescLength,
  OUT UINTN       *ParsedBytes
  )
{
  UINT16  DescriptorHeader;
  UINT8   Len;
  UINT8   *Ptr;
  UINTN   Parsed;

  Parsed  = 0;
  Ptr     = Buffer;

  while (TRUE) {
    //
    // Buffer length should not less than Desc length
    //
    if (Length < DescLength) {
      return EFI_DEVICE_ERROR;
    }

    DescriptorHeader  = (UINT16) (*Ptr + ((* (Ptr + 1)) << 8));

    Len               = Buffer[0];

    //
    // Check to see if it is a start of expected descriptor
    //
    if (DescriptorHeader == ((DescType << 8) | DescLength)) {
      break;
    }

    if ((UINT8) (DescriptorHeader >> 8) == DescType) {
      if (Len > DescLength) {
        return EFI_DEVICE_ERROR;
      }
    }
    //
    // Descriptor length should be at least 2
    // and should not exceed the buffer length
    //
    if (Len < 2) {
      return EFI_DEVICE_ERROR;
    }

    if (Len > Length) {
      return EFI_DEVICE_ERROR;
    }
    //
    // Skip this mismatch descriptor
    //
    Length -= Len;
    Ptr += Len;
    Parsed += Len;
  }

  *ParsedBytes = Parsed;

  return EFI_SUCCESS;
}

/**
  Send reset signal over the given root hub port.

  @param  PeiServices       Describes the list of possible PEI Services.
  @param  UsbHcPpi          The pointer of PEI_USB_HOST_CONTROLLER_PPI instance.
  @param  Usb2HcPpi         The pointer of PEI_USB2_HOST_CONTROLLER_PPI instance.
  @param  PortNum           The port to be reset.
  @param  RetryIndex        The retry times.

**/
VOID
ResetRootPort (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN PEI_USB_HOST_CONTROLLER_PPI    *UsbHcPpi,
  IN PEI_USB2_HOST_CONTROLLER_PPI   *Usb2HcPpi,
  IN UINT8                          PortNum,
  IN UINT8                          RetryIndex
  )
{
  EFI_STATUS             Status;
  UINTN                  Index;
  EFI_USB_PORT_STATUS    PortStatus;


  if (Usb2HcPpi != NULL) {
    MicroSecondDelay (200 * 1000);

    //
    // reset root port
    //
    Status = Usb2HcPpi->SetRootHubPortFeature (
               PeiServices,
               Usb2HcPpi,
               PortNum,
               EfiUsbPortReset
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SetRootHubPortFeature EfiUsbPortReset Failed\n"));
      return;
    }

    //
    // Drive the reset signal for at least 50ms. Check USB 2.0 Spec
    // section 7.1.7.5 for timing requirements.
    //
    MicroSecondDelay (USB_SET_ROOT_PORT_RESET_STALL);

    //
    // clear reset root port
    //
    Status = Usb2HcPpi->ClearRootHubPortFeature (
               PeiServices,
               Usb2HcPpi,
               PortNum,
               EfiUsbPortReset
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ClearRootHubPortFeature EfiUsbPortReset Failed\n"));
      return;
    }

    MicroSecondDelay (USB_CLR_ROOT_PORT_RESET_STALL);

    //
    // USB host controller won't clear the RESET bit until
    // reset is actually finished.
    //
    ZeroMem (&PortStatus, sizeof (EFI_USB_PORT_STATUS));

    for (Index = 0; Index < USB_WAIT_PORT_STS_CHANGE_LOOP; Index++) {
      Status = Usb2HcPpi->GetRootHubPortStatus (
                 PeiServices,
                 Usb2HcPpi,
                 PortNum,
                 &PortStatus
                 );
      if (EFI_ERROR (Status)) {
        return;
      }

      if (!USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_RESET)) {
        break;
      }

      MicroSecondDelay (USB_WAIT_PORT_STS_CHANGE_STALL);
    }

    if (Index == USB_WAIT_PORT_STS_CHANGE_LOOP) {
      DEBUG ((DEBUG_ERROR, "ResetRootPort: reset not finished in time on port %d\n", PortNum));
      return;
    }

    Usb2HcPpi->ClearRootHubPortFeature (
      PeiServices,
      Usb2HcPpi,
      PortNum,
      EfiUsbPortResetChange
      );

    Usb2HcPpi->ClearRootHubPortFeature (
      PeiServices,
      Usb2HcPpi,
      PortNum,
      EfiUsbPortConnectChange
      );

    //
    // Set port enable
    //
    Usb2HcPpi->SetRootHubPortFeature (
      PeiServices,
      Usb2HcPpi,
      PortNum,
      EfiUsbPortEnable
      );

    Usb2HcPpi->ClearRootHubPortFeature (
      PeiServices,
      Usb2HcPpi,
      PortNum,
      EfiUsbPortEnableChange
      );

    MicroSecondDelay ((RetryIndex + 1) * 50 * 1000);
  } else {
    MicroSecondDelay (200 * 1000);

    //
    // reset root port
    //
    Status = UsbHcPpi->SetRootHubPortFeature (
               PeiServices,
               UsbHcPpi,
               PortNum,
               EfiUsbPortReset
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SetRootHubPortFeature EfiUsbPortReset Failed\n"));
      return;
    }

    //
    // Drive the reset signal for at least 50ms. Check USB 2.0 Spec
    // section 7.1.7.5 for timing requirements.
    //
    MicroSecondDelay (USB_SET_ROOT_PORT_RESET_STALL);

    //
    // clear reset root port
    //
    Status = UsbHcPpi->ClearRootHubPortFeature (
               PeiServices,
               UsbHcPpi,
               PortNum,
               EfiUsbPortReset
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ClearRootHubPortFeature EfiUsbPortReset Failed\n"));
      return;
    }

    MicroSecondDelay (USB_CLR_ROOT_PORT_RESET_STALL);

    //
    // USB host controller won't clear the RESET bit until
    // reset is actually finished.
    //
    ZeroMem (&PortStatus, sizeof (EFI_USB_PORT_STATUS));

    for (Index = 0; Index < USB_WAIT_PORT_STS_CHANGE_LOOP; Index++) {
      Status = UsbHcPpi->GetRootHubPortStatus (
                 PeiServices,
                 UsbHcPpi,
                 PortNum,
                 &PortStatus
                 );
      if (EFI_ERROR (Status)) {
        return;
      }

      if (!USB_BIT_IS_SET (PortStatus.PortStatus, USB_PORT_STAT_RESET)) {
        break;
      }

      MicroSecondDelay (USB_WAIT_PORT_STS_CHANGE_STALL);
    }

    if (Index == USB_WAIT_PORT_STS_CHANGE_LOOP) {
      DEBUG ((DEBUG_ERROR, "ResetRootPort: reset not finished in time on port %d\n", PortNum));
      return;
    }

    UsbHcPpi->ClearRootHubPortFeature (
      PeiServices,
      UsbHcPpi,
      PortNum,
      EfiUsbPortResetChange
      );

    UsbHcPpi->ClearRootHubPortFeature (
      PeiServices,
      UsbHcPpi,
      PortNum,
      EfiUsbPortConnectChange
      );

    //
    // Set port enable
    //
    UsbHcPpi->SetRootHubPortFeature (
      PeiServices,
      UsbHcPpi,
      PortNum,
      EfiUsbPortEnable
      );

    UsbHcPpi->ClearRootHubPortFeature (
      PeiServices,
      UsbHcPpi,
      PortNum,
      EfiUsbPortEnableChange
      );

    MicroSecondDelay ((RetryIndex + 1) * 50 * 1000);
  }
  return;
}

/**
  Enumerate devices on the USB bus.
  It will call the callback function for each device enumerated.

  @param  UsbHostHandle     USB host controller handle.
  @param  UsbIoCb           Callback function for each USB device detected.

  @retval EFI_SUCCESS       The usb is enumerated successfully.
  @retval Others            Other failure occurs.

**/
EFI_STATUS
EFIAPI
UsbEnumBus (
  EFI_HANDLE                             UsbHostHandle,
  USB_IO_CALLBACK                        UsbIoCb
  )
{
  mUsbIoCb = UsbIoCb;
  return PeiUsbEnumeration ((EFI_PEI_SERVICES **) NULL, NULL, (PEI_USB2_HOST_CONTROLLER_PPI *)UsbHostHandle);
}

/**
  Free USB device memory.

  @param  UsbIo     UsbIo instance associated with the device.

  @retval EFI_SUCCESS            The usb device memory was de-allocated successfully.
  @retval EFI_INVALID_PARAMETER  Invalid UsbIo instance.

**/
EFI_STATUS
EFIAPI
UsbDeinitDevice (
  IN VOID     *UsbIo
  )
{
  PEI_USB_IO_PPI   *This;
  PEI_USB_DEVICE   *PeiUsbDev;
  UINTN             MemPages;

  if (UsbIo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  This = (PEI_USB_IO_PPI *)UsbIo;
  PeiUsbDev = PEI_USB_DEVICE_FROM_THIS (This);
  MemPages = sizeof (PEI_USB_DEVICE) / EFI_PAGE_SIZE + 1;
  FreePages (PeiUsbDev, MemPages);

  return EFI_SUCCESS;
}


/**
  Retrieve the concatenated manufacturer and product string for a given USB device.

  @param[in]  UsbIo for a USB device.

  @retval     The constant manufacturer and product string.
              NULL if UsbIo is not valid.

**/
CONST
CHAR16 *
EFIAPI
GetUsbDeviceNameString (
  IN PEI_USB_IO_PPI  *UsbIo
  )
{
  PEI_USB_IO_PPI   *This;
  PEI_USB_DEVICE   *PeiUsbDev;

  if (UsbIo == NULL) {
    return NULL;
  }

  This = (PEI_USB_IO_PPI *)UsbIo;
  PeiUsbDev = PEI_USB_DEVICE_FROM_THIS (This);

  if (PeiUsbDev->ProductName[0] == 0) {
    return NULL;
  } else {
    return PeiUsbDev->ProductName;
  }
}
