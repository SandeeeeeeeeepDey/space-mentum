/**
 * package-dist.js
 * Creates a standalone distribution package of the SpaceMentum 3D Companion Prototype.
 */

const fs = require('fs');
const path = require('path');

const srcDir = __dirname;
const distDir = path.join(srcDir, 'dist');

if (!fs.existsSync(distDir)) {
  fs.mkdirSync(distDir, { recursive: true });
}

const filesToCopy = ['index.html', 'app.js', 'style.css', 'package.json'];

filesToCopy.forEach(file => {
  const src = path.join(srcDir, file);
  const dest = path.join(distDir, file);
  if (fs.existsSync(src)) {
    fs.copyFileSync(src, dest);
    console.log(`[DIST] Copied ${file} -> dist/${file}`);
  }
});

console.log('[DIST] SpaceMentum Web Standalone Package built successfully in dist/ !');
