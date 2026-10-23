# CT2 publication and recovery

Target: [Psygoo/TrackerPlus-CT-Web](https://github.com/Psygoo/TrackerPlus-CT-Web).
Release tag: **v0.2.1-CT2**. [Release notes](RELEASE_NOTES.md).
The project owner explicitly authorized the source-only release and Pages
update after confirming the final CT2 firmware on physical hardware. Firmware
is never distributed or flashed by the release process.

Read-only remote checks found `gh-pages` at
`ca853c107ef3ddfdfa3251ea967f89da60d7816d`, with the published H12 tag/release
`v0.2.0-h12`. CT6 is archived at `archive/ct6-before-h12`, commit
`dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad`. Preserve both references, history and
availability. Re-fetch the remote and stop/review unexpected changes before a
fast-forward update; never force-push or rewrite history.

`PUBLIC_FILES.json` is the exact file allowlist and SHA-256 manifest (excluding
itself from recursion). Publish only that reviewed source tree or its source
archive, never the surrounding private development repository or evidence.
Runtime assets: `.nojekyll`, `index.html`, `documentation.html`, `style.css`,
`app.js`, `worker.js`, `session.js`, `patcher.js` and `ct2-program.js`.
Host over HTTPS; preserve `connect-src 'none'` and the local-only worker flow.

No PTF, dump, extracted proprietary function, stock instruction window, private
fixture, local evidence, SD content, toolchain, dependency cache or credential
belongs in publication. The generated template contains project code and zero
slots for 14 stock replay bytes, supplied only from a verified original input.
Preserve the original MIT license and the complete historical NOTICE.

CT2 reference/output identity, local Chromium/Firefox checks, firmware checks
and final owner-reported CT2 hardware validation are complete. After deployment,
verify the actual public site and generated hash before claiming success.
Do not alter, withdraw or overwrite the old H12 release.
