# Browser patcher architecture and reproducibility

No runtime libraries, telemetry, upload endpoints, external fonts or service
worker are used. Load the static files over HTTPS or localhost. File bytes go
from File API to a local module Worker, through PTF verification/patching, then
back to a verified Blob download. CSP sets `connect-src 'none'`; Web Crypto
checks original, decoded image, hook windows, project template and final PTF.
The UI independently checks the returned output hash before enabling download.

The source-generated template is 6,816 bytes, SHA-256
`ae08816f415fa769e0b7adf1a0ea6c2095f66d70d24c8fe5302e844c9b175118`.
It has 29 guarded prefix edits and three zero replay slots totaling 14 bytes.
Those slots receive instructions exclusively from the original user input.
The source compiler does not open any firmware file. Historical inactive
project code, padding and incremental Intel HEX record boundaries are retained
because the tested H12 file's exact identity includes them.

Rebuild using Python 3 and Arm GNU Toolchain **15.2.Rel1**:

`python3 tools/build-program.py --compiler /path/to/arm-gnu-toolchain/bin --check`

Regenerate readable documentation with `python3 tools/render-docs.py`.
Before packaging, run `python3 tools/audit.py --write-manifest` and then
`python3 tools/audit.py`; review `PUBLIC_FILES.json` and the tree diff.
The audit checks the explicit file allowlist, UTF-8 assets, fingerprints,
license/historical notice, local links, local paths and common credential forms.
It is a bounded source/package check, not a general legal or security audit.

Run the [README development commands](README.md) with private fixtures outside
the public/served directory. Full validation must compare browser output to
the actual H12 reference, not just an independently generated patch. The
browser test blocks requests during processing, checks invalid/stale input,
downloads twice through chooser/drop paths, and compares each output byte.

No publication or deployment script is included. See [PUBLICATION.md](PUBLICATION.md).
