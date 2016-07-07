
_Function_class_(IO_COMPLETION_ROUTINE)
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
static
NTSTATUS
NTAPI
VhciDeviceUsageNotificationCompletion(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp,
    _In_reads_opt_(_Inpressible_("varies")) PVOID Context)
{
    PVHCI_FDO_DEVICE_EXTENSION FdoExtension;

    UNREFERENCED_PARAMETER(Context);

    FdoExtension = DeviceObject->DeviceExtension;
    NT_ASSERT(FdoExtension->Common.Signature == VHCI_FDO_SIGNATURE);
    if (Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }

    if (!(FdoExtension->LowerDevice->Flags & DO_POWER_PAGABLE))
    {
        DeviceObject->Flags &= ~DO_POWER_PAGABLE;
    }

    return STATUS_CONTINUE_COMPLETION;
}
