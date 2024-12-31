

targets = [ "index.html", "script.js", "style.css" ]
output = "www_data.c"
variables = [ "index_html", "script_js", "style_css" ]

content = """#include \"www_data.h\"
"""

def append_file(filename, variablename):
    global content
    content += f"const char* {variablename} = \n"
    with open(filename, "r", encoding="utf-8") as f:
        for line in f.readlines():
            line = line.rstrip("\r\n")
            line = line.replace('\\', '\\\\')
            line = line.replace('"', '\\"')
            content += f'    "{line}\\r\\n"' + "\n"
    content += ";\n"

for i in range(len(targets)):
    filename = targets[i]
    variablename = variables[i]
    append_file(filename, variablename)


with open(output, "w", encoding="utf-8") as f:
    f.write(content)
