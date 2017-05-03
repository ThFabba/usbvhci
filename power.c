/*
* PROJECT:         ReactOS Virtual USB HCI Driver
* LICENSE:         LGPLv2.1+ - See COPYING.LIB in the top level directory
* PURPOSE:         Power functions
* PROGRAMMER:      Thomas Faber <thomas.faber@reactos.org>
*/

#include "usbvhci.h"

_Use_decl_annotations_
NTSTATUS
NTAPI
VhciDispatchPower(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    NTSTATUS Status;
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension;
    PIO_STACK_LOCATION IoStack;

    FdoExtension = DeviceObject->DeviceExtension;
    NT_ASSERT(FdoExtension->Common.DeviceObject == DeviceObject);
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    if (FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE)
    {
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        return PoCallDriver(FdoExtension->LowerDevice, Irp);
    }

    NT_ASSERT(FdoExtension->Common.Signature == VHCI_PDO_SIGNATURE);

    Status = Irp->IoStatus.Status;
    switch (IoStack->MinorFunction)
    {
        case IRP_MN_QUERY_POWER:
        case IRP_MN_SET_POWER:
            Status = STATUS_SUCCESS;
            break;
    }

    Irp->IoStatus.Status = Status;
    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}