# AGENTS.md

## Cursor Cloud specific instructions

This is a **zero-dependency static portfolio website** (HTML, CSS, vanilla JavaScript). There is no package manager, no build system, no framework, and no automated test suite.

### Running the dev server

Serve the site locally with any static HTTP server from the repository root:

```
python3 -m http.server 8080
```

Then open `http://localhost:8080/` in a browser.

### Linting / validation

- **JavaScript**: `node -c script.js` checks syntax.
- **HTML**: All `.html` files can be parsed with Python's `html.parser` for basic well-formedness checks.
- There is no ESLint, Prettier, or CSS linter configured in this repo.

### Key files

| File | Purpose |
|---|---|
| `index.html` | Homepage with hero + project carousel |
| `about.html` | About page |
| `runar.html` | RunAR case study |
| `slumberpal.html` | SlumberPal case study |
| `wordstream-research.html` | Research project page |
| `styles.css` | All styles (~1500 lines) |
| `script.js` | All JS (~510 lines) — navbar, carousel, cursor, transitions |
| `images/` | Image assets |

### Notes

- No build step is needed; edits to `.html`, `.css`, or `.js` are reflected on browser reload.
- `.cpanel.yml` is the production deployment config (copies files to `public_html/`). It is not relevant for local development.
- Some project cards link to external Unsplash images; those will not load without internet access but the site still functions.
