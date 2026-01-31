# Git Deployment to cPanel Guide

This guide will help you set up automatic deployment from your local files to cPanel using Git.

## Option 1: cPanel Git Version Control (Recommended)

Most modern cPanel hosts support Git deployment directly through the control panel.

### Step 1: Initialize Git Locally

1. **Open Terminal** in your project folder (`portoliot_test`)

2. **Initialize Git repository:**
   ```bash
   git init
   git add .
   git commit -m "Initial commit"
   ```

3. **Create a GitHub/GitLab/Bitbucket repository** (optional but recommended)
   - Go to GitHub.com (or GitLab/Bitbucket)
   - Create a new repository
   - Don't initialize with README
   - Copy the repository URL

4. **Connect local repo to remote:**
   ```bash
   git remote add origin YOUR_REPOSITORY_URL
   git branch -M main
   git push -u origin main
   ```

### Step 2: Set Up Git in cPanel

1. **Log into cPanel**
   - Navigate to your cPanel dashboard

2. **Find Git Version Control**
   - Look for "Git Version Control" or "Git™ Version Control" in cPanel
   - If you don't see it, contact your host - they may need to enable it

3. **Create Git Repository in cPanel**
   - Click "Create" or "Clone a Repository"
   - Repository Name: `portfolio` (or any name you prefer)
   - Repository Path: `/home/username/public_html` (or your domain folder)
   - Remote URL: Paste your GitHub/GitLab repository URL
   - Branch: `main` (or `master`)
   - Click "Create"

4. **Set Up Auto-Deploy (if available)**
   - Some hosts have "Auto Deploy" option
   - Enable it to automatically pull changes when you push to GitHub

5. **Manual Pull (if no auto-deploy)**
   - After pushing to GitHub, go back to cPanel Git Version Control
   - Click "Pull or Deploy" on your repository
   - This will update your live site

### Step 3: Workflow for Updates

1. **Make changes locally** in your files
2. **Commit changes:**
   ```bash
   git add .
   git commit -m "Description of changes"
   git push
   ```
3. **Deploy to cPanel:**
   - If auto-deploy is enabled: Changes go live automatically
   - If manual: Go to cPanel → Git Version Control → Pull or Deploy

---

## Option 2: FTP Sync with Git (Alternative)

If your cPanel doesn't support Git, you can use Git locally and sync via FTP.

### Using Git + FTP Sync Tools

1. **Use Git locally** for version control
2. **Use an FTP sync tool** to deploy:
   - **FileZilla**: Set up site manager, use "Synchronize directories"
   - **Cyberduck**: Has sync feature
   - **VS Code FTP Sync extension**: If you use VS Code

### VS Code FTP Sync Setup

1. Install "FTP-Sync" extension in VS Code
2. Create `.vscode/ftp-sync.json`:
   ```json
   {
     "protocol": "ftp",
     "host": "your-domain.com",
     "port": 21,
     "username": "your-ftp-username",
     "password": "your-ftp-password",
     "remotePath": "/public_html",
     "uploadOnSave": false,
     "passive": true
   }
   ```
3. Right-click files → "Upload" or use sync command

---

## Option 3: GitHub Actions + cPanel (Advanced)

For fully automated deployment:

1. **Set up GitHub Actions** workflow
2. **Use FTP deployment action** to push to cPanel
3. **Every push to main branch** automatically deploys

---

## Quick Setup Script

Run this in your terminal to set up Git:

```bash
cd /Users/jaimeserrano/Downloads/portoliot_test
git init
git add .
git commit -m "Initial portfolio website commit"
```

Then connect to your remote repository:
```bash
git remote add origin YOUR_GITHUB_REPO_URL
git push -u origin main
```

---

## Recommended Workflow

1. **Make changes** to your files locally
2. **Test locally** (open index.html in browser)
3. **Commit changes:**
   ```bash
   git add .
   git commit -m "Update about page content"
   git push
   ```
4. **Deploy to cPanel** (via Git Version Control or FTP sync)

---

## Troubleshooting

**Git not available in cPanel?**
- Contact your hosting provider to enable Git Version Control
- Or use FTP sync method instead

**Files not updating?**
- Check file permissions (should be 644 for files, 755 for folders)
- Verify you're pushing to the correct branch
- Clear browser cache

**Need help?**
- Check cPanel documentation for Git Version Control
- Contact your hosting provider's support
