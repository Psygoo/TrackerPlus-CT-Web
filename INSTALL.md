# Installation

This independent project is not affiliated with Polyend. Read the
[disclaimer](DISCLAIMER.md), [compatibility guide](COMPATIBILITY.md) and
[recovery procedure](RECOVERY.md) before making changes.

1. Obtain the official **Polyend Tracker+ 1.2.0** PTF from
   [Polyend's Tracker+ downloads](https://polyend.com/downloads/tracker-plus-downloads/).
   Extract the PTF if delivered in an archive. Use the plus model, not Mini,
   original Tracker, a beta, previously patched image or another version.
   Supported PTF SHA-256:
   `d4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd`.
   If the official download differs, stop: do not bypass the check.
2. Back up the **entire SD card**, including Workspace, projects, samples and
   settings, to a separate device. Keep untouched project copies, the original
   official firmware and a known working recovery image. Review CT6 migration
   before playing older conditional projects.
3. Open the reviewed web patcher on HTTPS, or serve this source locally with
   `node tools/serve.mjs`. Select/drop your own original PTF. Processing stays
   in browser memory; no upload occurs. Wait for successful verification.
4. Save `TrackerPlus_H12.ptf`. Its required SHA-256 is
   `40079d13a100c8c73953496c9bdebec034c6b86a1e99c2706e717fc8e0b18c79`.
   A hash mismatch or error means do not install. For an independent check,
   use `sha256sum TrackerPlus_H12.ptf` on Linux, `shasum -a 256` on macOS or
   `Get-FileHash TrackerPlus_H12.ptf -Algorithm SHA256` in PowerShell.
5. Follow the official manual's firmware update procedure: copy the generated
   PTF into the SD card's Firmware folder, safely eject, insert in Tracker+,
   then select the file under **Config → Firmware** and follow device prompts.
   Use reliable power and do not interrupt the update. The website never
   flashes a device or edits your SD card.
6. Confirm **1.2.0-h12 / Build 1504**, then test a copied project. Check the
   conditions and Fill modes you use before committing important work.

Manual reference: [Polyend Tracker+ manual, firmware update/recovery](https://polyend-website.fra1.digitaloceanspaces.com/wp-content/uploads/2024/05/08111709/Polyend-TrackerPlus-Manual-1v1v0c.pdf#page=318).
To return to official firmware, use its normal update procedure when available,
or read [recovery](RECOVERY.md). Restoring firmware does not migrate projects.
