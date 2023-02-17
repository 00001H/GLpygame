extiss = {
".hpp": "C++ Files",
".glsl": "Shaders",
".py": "Python scripts"
}
rstiss = dict.fromkeys(extiss.keys(),0)
lntiss = dict.fromkeys(extiss.keys(),0)
import os
files = 0
for cd,sd,sf in os.walk("."):
    if "Pack" in sd:
        sd.remove("Pack")
    for f in sf:
        for ky in extiss:
            if f.endswith(ky):
                contents = open(os.path.join(cd,f),"r").read()
                files += 1
                rstiss[ky] += len(contents.replace("\n","").replace(" ",""))
                lntiss[ky] += len(contents.splitlines())
                break
tchrs = sum(rstiss.values())
tlns = sum(lntiss.values())
print("--Work summary--")
print(f"You wrote {tchrs} characters({tlns} lines) of code across {files} files,")
for ext,extn in extiss.items():
    print(f"{rstiss[ext]} characters({lntiss[ext]} lines) in {ext}({extn}) files,")
print(f"and that's {tchrs//3000} headaches.")
