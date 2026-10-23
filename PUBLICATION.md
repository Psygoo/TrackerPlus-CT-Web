# Publication plan — approval required

This staged tree is the proposed replacement source/static tree for
[Psygoo/TrackerPlus-CT-Web](https://github.com/Psygoo/TrackerPlus-CT-Web).
Nothing has been pushed, deployed, tagged or publicly released by this task.
The existing CT6 website remains untouched.

Suggested release tag: **v0.2.0-h12**. Suggested release body:
[RELEASE_NOTES.md](RELEASE_NOTES.md). After explicit approval, retain the old CT6
commit `dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad` with a recoverable archival
reference before updating the existing gh-pages branch. Review the final diff,
including deletion of the obsolete CT6 program module, rather than overlaying
files blindly. Do not use the private development repository or its history as
the public source tree.

`PUBLIC_FILES.json` is the exact relative file allowlist and SHA-256 manifest
for publication, excluding itself to avoid recursion. It includes source,
tests, docs and static assets only. Development dependencies and browser caches
are not included. Static runtime files are `.nojekyll`, `index.html`,
`documentation.html`, `style.css`, `app.js`, `worker.js`, `session.js`,
`patcher.js` and `h12-program.js`. Serve these over HTTPS without analytics,
upload endpoints, external script injection or a service worker.

Do not publish any PTF (original or generated), firmware dump, proprietary
instruction window, private test fixture, vendor toolchain, browser binary,
SD/project content, local checkpoint or credential. Only zero-filled replay
slots and original-input fingerprints are in the public patch program.

The owner must explicitly approve publication of this concrete tree and release
notes before any push, deployment, GitHub release or live Pages update.
