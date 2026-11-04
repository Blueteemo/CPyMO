# CPyMO for NDS v1.2.1-test4 — approved final typography review

Based on the fully built v1.2.1-test3 candidate (commit 953cee7242650ea19e8425f88d4e04d3117c8c8e).
This test4 iteration changes **only the theme palette's typography and Default selection rail**.

- Default: 2-pixel selection rail RGB555 (16,27,31), the approved B variant; every other default role remains unchanged.
- Sixteen native IPL themes: approved A typography, RGB555 primary (30,30,30); accent theme_tint(13,17); muted theme_tint(3,23); detail theme_tint(3,24); clock theme_tint(2,24); auxiliary clock theme_tint(3,23). Existing colorful selection rail, surface background, header, card and highlight colors remain unchanged.
- All language choices, centered theme labels, automatic firmware caching, title-guide restart behavior, alpha rounding, and the test3 +1.5 dB guarded audio remain as before.

## Hardware validation reminders

Visually compare Default with a bright theme (Pink/Green/Red) and a dark theme (Dark Blue/Dark Purple). The default 2px selection rail should now have comparable contrast to the colored-theme rails. Main text should look neutral and clearly distinct from themed accent and secondary text. Confirm Auto remains responsive.
For confidence that this visual-only change did not break functionality, briefly verify in-game restart title guide and music/speech/SE/movie audio as previously tested.

Development TEST only. No release published, upstream PR #60 untouched, desktop converter icon not included.
