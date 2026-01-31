# cPanel Upload Guide

> **💡 Want automatic deployment?** See `GIT_DEPLOYMENT_GUIDE.md` for setting up Git-based automatic updates to cPanel.

## Files to Upload

Upload ALL of these files and folders to your cPanel hosting:

### Required Files:
- `index.html` (your homepage)
- `about.html`
- `runar.html`
- `styles.css`
- `script.js`
- `Jaime Serrano_12_2025.pdf`
- `RunAR_report.pdf` (optional, only if you want it accessible)

### Required Folders:
- `images/` folder (with ALL contents):
  - `cartoon-cat-cursor.svg`
  - `cartoon-mouse-cursor.svg`
  - `Frame.jpg`
  - `linkedin_pic.jpg`
  - All screenshot images (Screenshot 2026-01-31 at...)

## Step-by-Step Upload Instructions

### Method 1: Using cPanel File Manager (Recommended)

1. **Log into cPanel**
   - Go to your hosting provider's cPanel login page
   - Enter your username and password

2. **Open File Manager**
   - Find and click on "File Manager" in cPanel
   - Navigate to `public_html` folder (this is your website root)

3. **Upload Files**
   - Click "Upload" button in File Manager
   - Select all the files listed above
   - Wait for upload to complete

4. **Create Images Folder**
   - In File Manager, create a folder named `images` in `public_html`
   - Upload all image files into this folder (including all screenshot images)

5. **Verify Structure**
   Your `public_html` should look like this:
   ```
   public_html/
   ├── index.html
   ├── about.html
   ├── runar.html
   ├── styles.css
   ├── script.js
   ├── Jaime Serrano_12_2025.pdf
   ├── RunAR_report.pdf
   └── images/
       ├── cartoon-cat-cursor.svg
       ├── cartoon-mouse-cursor.svg
       ├── Frame.jpg
       ├── linkedin_pic.jpg
       ├── Screenshot 2026-01-31 at 00-29-51 RunAR_report.pdf.png
       ├── Screenshot 2026-01-31 at 00-30-03 RunAR_report.pdf.png
       └── (all other screenshot images)
   ```

### Method 2: Using FTP (Alternative)

1. **Get FTP Credentials**
   - In cPanel, go to "FTP Accounts"
   - Note your FTP hostname, username, and password

2. **Connect with FTP Client**
   - Use FileZilla, Cyberduck, or similar FTP client
   - Connect to your server using the credentials

3. **Upload Files**
   - Navigate to `public_html` directory
   - Upload all files maintaining the same folder structure

## Important Notes

✅ **All paths are already relative** - Your site uses relative paths (like `./about.html`, `images/Frame.jpg`) which will work on cPanel

✅ **No configuration needed** - This is a static website, so no server-side configuration is required

✅ **Test after upload** - Visit your domain to make sure everything loads correctly

## Troubleshooting

**Images not showing?**
- Check that the `images` folder is in the same directory as `index.html`
- Verify file names match exactly (case-sensitive on some servers)
- Check file permissions (should be 644 for files, 755 for folders)

**CSS/JS not loading?**
- Ensure `styles.css` and `script.js` are in the root directory with `index.html`
- Check browser console for 404 errors

**Links not working?**
- Make sure all HTML files are in the same directory
- Verify file names match exactly (including capitalization)

## Quick Checklist

- [ ] All HTML files uploaded to `public_html`
- [ ] `styles.css` and `script.js` uploaded to `public_html`
- [ ] `images` folder created in `public_html`
- [ ] All images uploaded to `images` folder (including all screenshot images)
- [ ] PDF files uploaded (if needed)
- [ ] Test website in browser
- [ ] Check all images load correctly
- [ ] Test all navigation links
- [ ] Verify RunAR page loads correctly
