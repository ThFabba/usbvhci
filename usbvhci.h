/*
 * PROJECT:         ReactOS Virtual USB HCI Driver
 * LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
 * PURPOSE:         WMI support functions
 * PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
 */

#ifndef _USBVHCI_USBVHCI_H_
#define _USBVHCI_USBVHCI_H_

#include <ntddk.h>
#include <usb.h>
#include <usbioctl.h>
#include <hubbusif.h>
#include <usbbusif.h>

/* Defines */
#ifdef __REACTOS__
//#define NDEBUG
#include <debug.h>
#else
//#define DPRINT(...) ((0, 0) || DbgPrint(__VA_ARGS__))
#define DPRINT DPRINT1
#define DPRINT1(...) DbgPrint(__VA_ARGS__)
#define UNIMPLEMENTED DPRINT1("%s is not implemented\n", __FUNCTION__)
#endif

#define VHCI_FDO_SIGNATURE 'dFhV'
#define VHCI_PDO_SIGNATURE 'dPhV'

#define VHCI_PNP_TAG 'nPhV'

/* Types */
typedef struct _VHCI_COMMON_DEVICE_EXTENSION
{
    ULONG Signature;
    PDEVICE_OBJECT DeviceObject;
    BOOLEAN Deleted;
} VHCI_COMMON_DEVICE_EXTENSION, *PVHCI_COMMON_DEVICE_EXTENSION;

typedef struct _VHCI_FDO_DEVICE_EXTENSION
{
    VHCI_COMMON_DEVICE_EXTENSION Common;
    PDEVICE_OBJECT LowerDevice;
    struct _VHCI_PDO_DEVICE_EXTENSION *PdoExtension;
} VHCI_FDO_DEVICE_EXTENSION, *PVHCI_FDO_DEVICE_EXTENSION;

typedef struct _VHCI_PDO_DEVICE_EXTENSION
{
    VHCI_COMMON_DEVICE_EXTENSION Common;
    struct _VHCI_FDO_DEVICE_EXTENSION *FdoExtension;
    _Interlocked_
    volatile LONG InterfaceRefCount;
    BOOLEAN Present;
} VHCI_PDO_DEVICE_EXTENSION, *PVHCI_PDO_DEVICE_EXTENSION;

/* Globals */
extern PDRIVER_OBJECT VhciDriverObject;

/* businterface.c */
USB_BUSIFFN_CREATE_USB_DEVICE VhciHubCreateUsbDevice;
USB_BUSIFFN_INITIALIZE_USB_DEVICE VhciHubInitializeUsbDevice;
USB_BUSIFFN_GET_USB_DESCRIPTORS VhciHubGetUsbDescriptors;
USB_BUSIFFN_REMOVE_USB_DEVICE VhciHubRemoveUsbDevice;
USB_BUSIFFN_RESTORE_DEVICE VhciHubRestoreUsbDevice;
USB_BUSIFFN_GET_POTRTHACK_FLAGS VhciHubGetPortHackFlags;
USB_BUSIFFN_GET_DEVICE_INFORMATION VhciHubQueryDeviceInformation;

PUSB_BUSIFFN_GETUSBDI_VERSION GetUSBDIVersion;
PUSB_BUSIFFN_QUERY_BUS_TIME QueryBusTime;
PUSB_BUSIFFN_SUBMIT_ISO_OUT_URB SubmitIsoOutUrb;
PUSB_BUSIFFN_QUERY_BUS_INFORMATION QueryBusInformation;
PUSB_BUSIFFN_IS_DEVICE_HIGH_SPEED IsDeviceHighSpeed;

/* fdo.c */
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
_Success_(TRUE)
NTSTATUS
VhciForwardIrpAndForget(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    _Inout_ __drv_aliasesMem PIRP Irp);

_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_same_
NTSTATUS
VhciForwardIrpAndWait(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    _In_ PIRP Irp);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
VOID
VhciPdoRemoveDevice(
    _In_ _At_(PdoExtension->Common.DeviceObject, __drv_freesMem(Mem)) PVHCI_PDO_DEVICE_EXTENSION PdoExtension);

_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
NTSTATUS
NTAPI
VhciFdoPnp(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension,
    _Inout_ PIRP Irp);

/* pdo.c */
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
NTSTATUS
NTAPI
VhciPdoPnp(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _Inout_ PIRP Irp);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
NTSTATUS
NTAPI
VhciPdoDeviceControl(
    _In_ PVHCI_PDO_DEVICE_EXTENSION PdoExtension,
    _Inout_ PIRP Irp);

_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
NTSTATUS
VhciCreatePdo(
    _In_ PVHCI_FDO_DEVICE_EXTENSION FdoExtension);

/* power.c */
_Dispatch_type_(IRP_MJ_POWER)
DRIVER_DISPATCH_RAISED VhciDispatchPower;

/* wmi.h */
_Dispatch_type_(IRP_MJ_SYSTEM_CONTROL)
DRIVER_DISPATCH_PAGED VhciDispatchSystemControl;

#endif /* _USBVHCI_USBVHCI_H_ */
