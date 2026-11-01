#!/usr/bin/env python3
"""Create/check the immutable SHA-256 inventory for the public WR2 source snapshot."""
from pathlib import Path
import hashlib, json, sys

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'WR2_SOURCE_INVENTORY.json'
PAYLOAD_SOURCE = ROOT / 'wr2/payload-src'
FILES = [
    'LICENSE', 'NOTICE.md', 'README.md', 'COMPATIBILITY.md', 'WR2.md', 'WR2_QUICK_START.md', 'WR2_USER_GUIDE.md', 'quick-start.html', 'documentation.html',
    'index.html', 'style.css', 'app.js', 'worker.js', 'worker-core.js', 'session.js',
    'patcher.js', 'ct2-program.js', 'wr2-patcher.js',
    'tools/export-wr2-payloads.py', 'tools/wr2-source-inventory.py', 'tools/render-docs.py',
    'tests/core.test.mjs', 'tests/browser.mjs', 'package.json',
]
FILES += [str(p.relative_to(ROOT)) for p in sorted(PAYLOAD_SOURCE.rglob('*')) if p.is_file()]
FILES += [str(p.relative_to(ROOT)) for p in sorted((ROOT / 'wr2/generated').rglob('*')) if p.is_file()]

COMPONENTS = {
    'wr1': 'retained WR01 compatibility extension',
    'ui': 'Stage 9 playback and group-editing UI',
    'address': 'Stage 9 multi-instrument addressing correction',
    's11b': 'S11B project persistence baseline',
    's12a': 'S12A Instrument Copy and replacement fence',
    's12b': 'S12B companion banks and PCM binding',
    's12c': 'S12C cross-project Instrument Import',
    's13d': 'S13D hardware-calibrated white pad LED brightness overlay',
}
TARGET_BASE = 'ecf53fc0aef676de9557688076ae3e71baa898d9'
INVENTORY = {
    'format': 'psygoo-wr2-source-inventory-v1',
    'target': {
        'repository': 'Psygoo/TrackerPlus-CT-Web',
        'branch': 'codex/ct2-wr2-integration',
        'base_commit': TARGET_BASE,
        'source_commit': '9f3323bb24953a69d1d43ccb415eadd5bed86b1f',
        'snapshot_status': 'local documentation-structure candidate based on the published S13D source; firmware code and payloads are unchanged',
        'anchor': 'PUBLIC_FILES.json hashes this inventory; the containing Psygoo repository commit anchors the reviewed release snapshot.',
    },
    'authorship_and_licensing': {
        'conditional_trigs': 'Psygoo originated the Conditional Trigs concept, roadmap and CT6 implementation. Existing external MIT browser/patcher reuse remains attributed in NOTICE.md and LICENSE.',
        'wr2': 'Original Psygoo WR2 project work across WR01, Stage 9, S11B, S12A, S12B, S12C and the S13D pad LED calibration, per project-owner attribution. The local Modular-Trackular workspace is a technical-reference/evidence location; its path and untracked state do not establish external authorship.',
        'generated_payloads': 'Compilation output of the inventoried WR2 and S13D project sources plus symbol/settings metadata. Not a complete firmware image and contains no copied stock/native bytes.',
        'redistribution_review': 'The repository MIT license is retained. No conflicting license notice or imported third-party source file was found in the inventoried WR2 sources. Polyend reference sources used during research are not included.',
        'unresolved_files': [],
    },
    'source_lineage': [
        {'component': name, 'path_prefix': f'wr2/payload-src/{folder}/', 'attribution': 'Psygoo WR2 project work'}
        for folder, name in COMPONENTS.items()
    ] + [
        {'component': 'browser generator and build tooling', 'paths': ['wr2-patcher.js', 'worker-core.js', 'tools/export-wr2-payloads.py'], 'attribution': 'Psygoo CT2/WR2 Web Patcher integration'},
    ],
    'revision_identifiers': {
        'target_base_commit': TARGET_BASE,
        'ct2_release_commit': '4d95ba1149b8548efe0379ce7f836faa90c8308c',
        'reused_ct6_browser_patcher_commit': 'dfb5cd4bdfe3f8bd1c68c4c998c2d44365a596ad',
        'external_reference_workspace_head': 'b7cb5ec86e855c922aab699f3deea2fb8e244bc6',
        'wr2_stage_commits': None,
        'wr2_stage_commit_note': 'Stage directories are local staged project work not tracked by that external workspace commit. Their sources are instead pinned by this inventory and by the S12C acceptance input manifest.',
    },
    'verified_output_identities': {
        'stock_input_sha256': 'd4616304f33d6d0b56516c658f34fa22517c1dd1630021c1ca89afca7b618ccd',
        'ct2_encoded_sha256': 'baa93d46208c0f6ab2ca0c7507fb3c430703a3ceada582a780779844a0d03b20',
        'ct2_decoded_image_sha256': 'e7fdba8c3bdbb26f1a9500fabfc556b71c818bad0f027fb0b84e27c5718788be',
        'ct2_wr2_s13d_encoded_sha256': '0bdc7dfc32f5c98ebf8a3146811a87dc9fbf8a60a324ad9edc1a64cea7609c1a',
        'ct2_wr2_s13d_decoded_image_sha256': '854363d8ecb0c04fb2f7ed7017a6fb8e1a6d97e86ea683c4c94c90fbd92160b1',
        's12c_recovery_encoded_sha256': '06adca54ff28ae704e92c4464a975f3c0e34f16d67a177db53157895be199445',
        's12c_recovery_decoded_image_sha256': '931acd25fe374c678456e590346447311047678d6d934055cd49d5e1d6df8659',
    },
    'reference_evidence': [
        {'artifact': 'Phase 1 direct-stock source recipe', 'path': 'private phase1 source package/build.py (not exported)', 'sha256': '8655898dff03f50a1377f82df0a6bb1d8c6bcc2d4439ffe55b601e9817880b97', 'role': 'Defines the direct-stock stage composition used by the browser port.'},
        {'artifact': 'S12C stage builder', 'path': 'Modular-Trackular experiment STAGE12C_INSTRUMENT_IMPORT/build.py', 'sha256': '353dd66487e33413ced19f04954aaba31920a2461bd318a09747925dfb51ff87', 'role': 'Pins S12C source input hashes in its acceptance data.'},
        {'artifact': 'S12C software acceptance manifest', 'path': 'Modular-Trackular experiment STAGE12C_INSTRUMENT_IMPORT/results/acceptance.json', 'sha256': 'a9c60e69e5146e278e9b55366a24e1dd23892224bcc51bcb0984e2199a6a6a29', 'role': '38/38 software gates and exact S12C overlay/source-input evidence.'},
        {'artifact': 'S12C stage report', 'path': 'Modular-Trackular experiment STAGE12C_INSTRUMENT_IMPORT/REPORT.md', 'sha256': '184f9b0d0a07be9a06a90f150a8b0066f77477ad127655869f052eed2d29a23c', 'role': 'Describes S12C code lineage, test scope and limits.'},
        {'artifact': 'S12D physical acceptance/performance report', 'path': 'Modular-Trackular experiment STAGE12D_IMPORT_PERFORMANCE_DIAGNOSIS/REPORT.md', 'sha256': '8d0fb339322d74f9776d359d43e26d5247644fda37f5db660585cb45fb42bcdc', 'role': 'Records owner-reported success of all eight S12C physical acceptance scenarios and their evidence limits.'},
        {'artifact': 'S13D LED calibration report', 'path': 'Modular-Trackular experiment STAGE13D_LED_BRIGHTNESS_CALIBRATION/REPORT.md', 'sha256': '8c90fd2122d9fb67ac457b0737a4246fd2ba090064d9c5600343543f18b193da', 'role': 'Defines the calibrated brightness roles and identifies the S12C recovery baseline; owner hardware confirmation is recorded in PROJECT_CONTEXT v1.7.'},
        {'artifact': 'S13D build map', 'path': 'Modular-Trackular experiment STAGE13D_LED_BRIGHTNESS_CALIBRATION/TrackerPlus_WR2S13D.map.json', 'sha256': '65b9461e8b9629631cc82ae02d418c4d8db9c191db0eafcb6587f8a34d43e92e', 'role': 'Pins the 1,040-byte source-built LED overlay and exact hook sites used by the browser composition.'},
        {'artifact': 'MIT license comparison', 'path': 'LICENSE in target and external reference workspace', 'sha256': '080a0c2c2f4481f060f9ee3486137d9e7512a78cfbabecb31300cc275b837f97', 'role': 'The license texts match byte-for-byte; the original notice is preserved.'},
    ],
}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def render():
    rows = []
    for rel in sorted(set(FILES)):
        path = ROOT / rel
        if not path.is_file():
            raise SystemExit(f'missing inventory input: {rel}')
        component = 'generated or integration support'
        for folder, name in COMPONENTS.items():
            if rel.startswith(f'wr2/payload-src/{folder}/'):
                component = name
                break
        if rel.startswith('wr2/generated/'):
            component = 'generated project payload/guard metadata'
        rows.append({'path': rel, 'bytes': path.stat().st_size, 'sha256': sha(path), 'component': component})
    result = dict(INVENTORY)
    result['files'] = rows
    result['file_count'] = len(rows)
    return json.dumps(result, indent=2, sort_keys=False) + '\n'

if __name__ == '__main__':
    expected = render()
    if '--write' in sys.argv:
        OUT.write_text(expected)
        print(f'WROTE {OUT.name} {hashlib.sha256(expected.encode()).hexdigest()}')
    else:
        if not OUT.is_file() or OUT.read_text() != expected:
            raise SystemExit('WR2 source inventory is stale; review sources and run --write')
        print(f'WR2_SOURCE_INVENTORY_PASS {hashlib.sha256(expected.encode()).hexdigest()} ({len(json.loads(expected)["files"])} files)')
