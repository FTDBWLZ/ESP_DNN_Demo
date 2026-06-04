# ESP-DNN TFT AI Demo Cluster

Standalone copy of the four TFT AI demos selected by `main.c`.

Default app:

- `ACTIVE_APP_BODY_DETECT_TFT`

Build:

```powershell
cmd /c tools\idf_env.cmd idf.py build
```

Switch app at build time:

```powershell
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=1 reconfigure build
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=2 reconfigure build
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=3 reconfigure build
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=4 reconfigure build
```

Mapping:

- `1`: color code TFT
- `2`: palm keypoint TFT
- `3`: face detect TFT
- `4`: body detect TFT

If this project is built directly with the shared IDF environment, use:

```powershell
cmd /c D:\IDF_PRO\IDF_PROJECT\tools\idf_env.cmd idf.py build
```
