# bootguard-status

Print the Intel Boot Guard state of the running system, decoded from
`MSR_BOOT_GUARD_SACM_INFO` (MSR 0x13a).

It reports whether Boot Guard is enabled and, if so, whether the boot was
verified, measured, or both.

## Usage

```
# bootguard-status
```

Requires root and `/dev/cpu/0/msr`, which needs `CONFIG_X86_MSR`. Run
`modprobe msr` if the driver is built as a module.

## Example

```
SACM_INFO:       0x0000000300000050 (MSR 0x13a)
Boot Guard:      enabled
Verified boot:   yes
Measured boot:   no
TPM (per ACM):   none
NEM:             disabled
Force anchor:    yes
Key revoked:     no
Undecoded bits:  0x0000000200000000
```

Bits whose meaning is not known are printed as raw values.

On systems with Boot Guard-capable silicon but no profile fused, it prints
`Boot Guard: not enabled (no profile fused)` and stops there.

## How it works

The startup ACM records what it did in MSR 0x13a before handing control to
the IBB. The register remains readable at runtime.

The tool reads eight bytes from offset `0x13a` in `/dev/cpu/0/msr` and
decodes the fields. It does not use MEI/HECI, access the SPI flash, or write
anything to the system.

`TPM (per ACM)` refers to the TPM the ACM found and used, not simply whether
the system has a TPM. For example, a system with a working TPM 2.0 still
reports `none` when measured boot is disabled, since the ACM did not use it.

## License

BSD-2-Clause
