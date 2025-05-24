# HarmonyScape - Build & Deploy Quick Reference

## 🚀 For AI Assistants: How to Build and Deploy

When user requests "build and deploy" or "deploy":

```bash
cd /Users/paulparker/projects/vstproject/HarmonyScape/build
./build_and_deploy.sh
```

**That's it!** The script handles everything automatically.

## 📖 Full Documentation

See `HarmonyScape_Build_Deploy_Guide.md` for comprehensive details.

## 🎯 Key Points

- **Script Location**: `/Users/paulparker/projects/vstproject/HarmonyScape/build/build_and_deploy.sh`
- **Auto-increments version** and **generates unique build color**
- **Builds both VST3 and AU** formats
- **Deploys to system plugin directories**
- **Clears DAW plugin cache** for immediate recognition

## ✅ Success Indicator

Look for:
```
🎉 DEPLOYMENT COMPLETE!
Version: 1.0.XXXX
Build ID Color: R=X.X G=X.X B=X.X
``` 