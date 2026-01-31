# Quick Start: Git Deployment to cPanel

Follow these steps to set up automatic deployment from your local files to cPanel.

## Step 1: Initialize Git Locally

Open Terminal and navigate to your project folder, then run:

```bash
cd /Users/jaimeserrano/Downloads/portoliot_test

# Option A: Run the setup script
chmod +x setup-git.sh
./setup-git.sh

# Option B: Or run commands manually:
git init
git add .
git commit -m "Initial commit: Portfolio website with RunAR project"
git branch -M main
```

## Step 2: Create GitHub Repository

1. Go to [GitHub.com](https://github.com) and sign in
2. Click the **"+"** icon → **"New repository"**
3. Repository name: `portfolio-website` (or any name you prefer)
4. Description: "My portfolio website"
5. Choose **Public** or **Private**
6. **DO NOT** check "Initialize with README" (we already have files)
7. Click **"Create repository"**

## Step 3: Connect Local Repository to GitHub

After creating the repository, GitHub will show you commands. Run these in Terminal:

```bash
# Replace YOUR_USERNAME and YOUR_REPO_NAME with your actual values
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git
git push -u origin main
```

You'll be prompted for your GitHub username and password (or use a Personal Access Token).

## Step 4: Set Up Git in cPanel

1. **Log into cPanel**
   - Go to your hosting provider's cPanel login

2. **Find Git Version Control**
   - Look for "Git Version Control" or "Git™ Version Control" in cPanel
   - If you don't see it, contact your host to enable it

3. **Create Git Repository in cPanel**
   - Click **"Create"** or **"Clone a Repository"**
   - **Repository Name:** `portfolio` (or any name)
   - **Repository Path:** `/home/yourusername/public_html` 
     - (Replace `yourusername` with your cPanel username)
   - **Remote URL:** `https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git`
   - **Branch:** `main`
   - Click **"Create"**

4. **Set Up Auto-Deploy (if available)**
   - Look for **"Auto Deploy"** or **"Pull on Push"** option
   - Enable it so changes deploy automatically when you push to GitHub

5. **Initial Pull**
   - Click **"Pull or Deploy"** to pull your files from GitHub
   - Your website should now be live!

## Step 5: Workflow for Future Updates

Whenever you make changes:

1. **Edit files locally** in your project folder

2. **Commit and push changes:**
   ```bash
   git add .
   git commit -m "Description of your changes"
   git push
   ```

3. **Deploy to cPanel:**
   - **If auto-deploy enabled:** Changes go live automatically! 🎉
   - **If manual:** Go to cPanel → Git Version Control → Click "Pull or Deploy"

## Troubleshooting

**Can't find Git Version Control in cPanel?**
- Contact your hosting provider - they may need to enable it
- Some hosts call it "Git™ Version Control" or "Git Manager"

**Auto-deploy not working?**
- Check that the repository path points to `public_html`
- Verify the branch name matches (usually `main`)
- Try manual pull first to test

**Files not updating?**
- Make sure you're pushing to the correct branch
- Check file permissions in cPanel File Manager (should be 644 for files, 755 for folders)

**Need help?**
- See `GIT_DEPLOYMENT_GUIDE.md` for detailed instructions
- Contact your hosting provider's support

## Quick Reference Commands

```bash
# Make changes, then:
git add .
git commit -m "Your commit message"
git push

# Check status:
git status

# See commit history:
git log
```

---

**That's it!** Once set up, you can update your website by simply pushing to GitHub. 🚀
