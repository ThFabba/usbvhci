/*
* PROJECT:         ReactOS Virtual USB HCI Driver
* LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
* PURPOSE:         Driver entry functions
* PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
*/

#include "usbvhci.h"

/* Section assignment */
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VhciHubCreateUsbDevice)
#pragma alloc_text(PAGE, VhciHubInitializeUsbDevice)
#pragma alloc_text(PAGE, VhciHubGetUsbDescriptors)
#pragma alloc_text(PAGE, VhciHubRemoveUsbDevice)
#pragma alloc_text(PAGE, VhciHubRestoreUsbDevice)
#pragma alloc_text(PAGE, VhciHubGetPortHackFlags)
#pragma alloc_text(PAGE, VhciHubQueryDeviceInformation)
#endif /* ALLOC_PRAGMA */

/* Functions */
_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubCreateUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE *NewDeviceHandle,
    PUSB_DEVICE_HANDLE HubDeviceHandle,
    USHORT PortStatus,
    USHORT PortNumber)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(NewDeviceHandle);
    DBG_UNREFERENCED_PARAMETER(HubDeviceHandle);
    DBG_UNREFERENCED_PARAMETER(PortStatus);
    DBG_UNREFERENCED_PARAMETER(PortNumber);
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubInitializeUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubGetUsbDescriptors(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle,
    PUCHAR DeviceDescriptorBuffer,
    PULONG DeviceDescriptorBufferLength,
    PUCHAR ConfigDescriptorBuffer,
    PULONG ConfigDescriptorBufferLength)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    DBG_UNREFERENCED_PARAMETER(DeviceDescriptorBuffer);
    DBG_UNREFERENCED_PARAMETER(DeviceDescriptorBufferLength);
    DBG_UNREFERENCED_PARAMETER(ConfigDescriptorBuffer);
    DBG_UNREFERENCED_PARAMETER(ConfigDescriptorBufferLength);
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubRemoveUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle, 
    ULONG Flags)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    DBG_UNREFERENCED_PARAMETER(Flags);
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubRestoreUsbDevice(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE OldDeviceHandle,
    PUSB_DEVICE_HANDLE NewDeviceHandle)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(OldDeviceHandle);
    DBG_UNREFERENCED_PARAMETER(NewDeviceHandle);
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubGetPortHackFlags(
    PVOID BusContext,
    PULONG Flags)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(Flags);
    return STATUS_NOT_IMPLEMENTED;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciHubQueryDeviceInformation(
    PVOID BusContext,
    PUSB_DEVICE_HANDLE DeviceHandle,
    PVOID DeviceInformationBuffer,
    ULONG DeviceInformationBufferLength,
    PULONG LengthOfDataCopied)
{
    DBG_UNREFERENCED_PARAMETER(BusContext);
    DBG_UNREFERENCED_PARAMETER(DeviceHandle);
    DBG_UNREFERENCED_PARAMETER(DeviceInformationBuffer);
    DBG_UNREFERENCED_PARAMETER(DeviceInformationBufferLength);
    DBG_UNREFERENCED_PARAMETER(LengthOfDataCopied);
    return STATUS_NOT_IMPLEMENTED;
}
