# Licensing

This packaging repo's own scripts are MIT licensed, as shown in `LICENSE.txt`.

The OpenCV source bundled into wheels is Apache-2.0.

Important bundled binary/license notes from the README:

- wheels ship with FFmpeg under LGPLv2.1
- non-headless Linux wheels ship with Qt 5 under LGPLv3
- other bundled binary dependency licenses are listed in `LICENSE-3RD-PARTY.txt`

The README says patented/non-free algorithms such as SURF are not included in distributed wheels. SIFT is included for OpenCV versions where the patent has expired.

For bugs in OpenCV functions/classes themselves, the upstream OpenCV repository is the right place. This repo is mainly for Python wheel packaging, install/import problems, missing packaging functionality, and build/release issues.

