# CT2 browser patcher and source reproducibility

The verified CT6/H12 local-worker infrastructure is reused. File API → module
Worker → original SHA/layout/PTF checks → project recipe → full output SHA and
PTF checks → independent UI hash check → local Blob download. There is no upload
endpoint, runtime network API, analytics, external library/font or service worker.
CSP sets `connect-src 'none'`. Use HTTPS or localhost, not direct file URLs.

The CT2 template contains 6,850 bytes of rebuilt MIT project code/padding,
29 guarded original-prefix edits and three zero replay slots totaling 14 bytes.
Original bytes enter those slots only from the verified user's firmware input.
The new `src/ne_pre` component supplies H13's 34-byte PRE commit correction.
Existing component addresses and intermediate PTF record boundaries are retained.

`python3 tools/build-program.py --compiler /path/to/arm-gnu-toolchain/bin --check`
uses Arm GNU 15.2.Rel1 and does not read firmware. Regenerate documentation
using `python3 tools/render-docs.py`. Audit and seal the file list using
`python3 tools/audit.py --write-manifest`, then `python3 tools/audit.py`.

Tests in `tests/` accept private fixtures outside the served/source directory.
The browser test disables requests after imports, checks chooser/drop downloads
against the actual CT2 reference, verifies rejection/stale output/CSP and inspects
responsive layout. Browser caches, binaries and evidence are never included.
This is a bounded source/package audit, not a general legal/security certification.
See [verification](VALIDATION.md), [NOTICE](NOTICE.md) and [publication plan](PUBLICATION.md).
