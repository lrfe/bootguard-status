/* SPDX-License-Identifier: BSD-2-Clause */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MSR_BOOT_GUARD_SACM_INFO	0x13aU

/* Field layout, verified on Whiskey Lake (8th gen). */
#define SACM_NEM_ENABLED	(1ull << 0)	/* Non-Eviction Mode */
#define SACM_TPM_TYPE_SHIFT	1
#define SACM_TPM_TYPE_MASK	0x3ull
#define SACM_TPM_SUCCESS	(1ull << 3)
#define SACM_FACB		(1ull << 4)	/* Force Anchor Boot */
#define SACM_MEASURED		(1ull << 5)
#define SACM_VERIFIED		(1ull << 6)
#define SACM_REVOKED		(1ull << 7)
#define SACM_BTG_CAPABILITY	(1ull << 32)

#define SACM_KNOWN		(SACM_NEM_ENABLED | \
				 (SACM_TPM_TYPE_MASK << SACM_TPM_TYPE_SHIFT) | \
				 SACM_TPM_SUCCESS | SACM_FACB | SACM_MEASURED | \
				 SACM_VERIFIED | SACM_REVOKED | SACM_BTG_CAPABILITY)

static const char *const tpm_type[] = {
	"none",
	"TPM 1.2 (discrete)",
	"TPM 2.0 (discrete)",
	"Intel PTT (firmware TPM)",
};

static const char *yesno(uint64_t v)
{
	return v ? "yes" : "no";
}

static int read_msr(uint32_t msr, uint64_t *val)
{
	const char *path = "/dev/cpu/0/msr";
	ssize_t n;
	int fd;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		if (errno == ENOENT)
			fprintf(stderr, "%s: no such file. Load the msr module (modprobe msr)\n",
				path);
		else if (errno == EACCES || errno == EPERM)
			fprintf(stderr, "%s: permission denied. Run as root\n", path);
		else
			fprintf(stderr, "%s: %s\n", path, strerror(errno));
		return -1;
	}

	/* The msr driver maps each MSR number to a file offset. */
	n = pread(fd, val, sizeof(*val), msr);
	if (n < 0) {
		if (errno == EIO)
			fprintf(stderr, "MSR 0x%x is not implemented by this CPU\n", msr);
		else
			fprintf(stderr, "%s: read MSR 0x%x: %s\n", path, msr, strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int main(void)
{
	uint64_t v, unknown;

	if (read_msr(MSR_BOOT_GUARD_SACM_INFO, &v) < 0)
		return 1;

	printf("SACM_INFO:       0x%016llx (MSR 0x%x)\n",
	       (unsigned long long)v, MSR_BOOT_GUARD_SACM_INFO);

	if (!(v & SACM_BTG_CAPABILITY)) {
		printf("Boot Guard:      not enabled (no profile fused)\n");
		return 0;
	}

	printf("Boot Guard:      enabled\n");
	printf("Verified boot:   %s\n", yesno(v & SACM_VERIFIED));
	printf("Measured boot:   %s\n", yesno(v & SACM_MEASURED));
	printf("TPM (per ACM):   %s%s\n",
	       tpm_type[(v >> SACM_TPM_TYPE_SHIFT) & SACM_TPM_TYPE_MASK],
	       (v & SACM_TPM_SUCCESS) ? ", startup ok" : "");
	printf("NEM:             %s\n", (v & SACM_NEM_ENABLED) ? "enabled" : "disabled");
	printf("Force anchor:    %s\n", yesno(v & SACM_FACB));
	printf("Key revoked:     %s\n", yesno(v & SACM_REVOKED));

	unknown = v & ~SACM_KNOWN;
	if (unknown)
		printf("Undecoded bits:  0x%016llx\n", (unsigned long long)unknown);

	return 0;
}
