import os
import re
import sys

try:
    import numpy as np
except ImportError:
    subprocess.run([sys.executable, '-m', 'pip', 'install', 'numpy'])
    import numpy as np

def bytes_to_c_arr(data):
    return [format(b, '#04x') for b in data]


def read_file(file_name):
    data = np.fromfile(file_name, dtype='uint8')
    data = bytearray(data)
    return data


def create_array(name, data):
    size = str(len(data))
    static_content = f"\t\tstatic const uint64_t size = {size};\n\t\tconstexpr static const unsigned char data[" + size + "] ="
    array_content = "{{{}}}\n".format(", ".join(bytes_to_c_arr(data)))
    array_content = re.sub("(.{72})", "\t\t\t\\1\n", array_content, 0, re.DOTALL)
    array_content = '\n' + array_content
    final_content = static_content + array_content + ";"
    return final_content


def create_header(path, export_path):
    if not os.path.exists(path):
        print(f"Path does not exist: {path}")
        return
    if not os.path.isfile(path):
        print(f"Path is not a file: {path}")
        return

    # file = open(path, "rb")
    # binary_data = file.read()
    # file.close()

    filename, file_extension = os.path.splitext(os.path.basename(path))
    file_extension = file_extension[1:]

    header_name = f"{filename}{file_extension}"
    header_include_guard = "CODEGEN_" + header_name.upper() + "_H"

    if not os.path.exists(f"{export_path}/EmbedResources"):
        os.mkdir(f"{export_path}/EmbedResources")

    headerfile = open(f"{export_path}/EmbedResources/{header_name}.h", "w")

    header_contents = ""
    header_contents += "/// This file is created by ResourceEmbedder.py\n\n"
    header_contents += f"#ifndef {header_include_guard}\n"
    header_contents += f"#define {header_include_guard}\n\n"
    header_contents += "#include <cstdint>\n\n"
    header_contents += "namespace ResourceEmbedder::Resources {\n"
    header_contents += f"\tclass {header_name} " + " {\n"
    header_contents += "\tpublic:\n"
    header_contents += f"\t\t{header_name}() = delete;\n"

    header_contents += create_array(header_name, read_file(path))

    """
    header_contents += f"\tconst unsigned char {header_name}[] = "
    header_contents += "{\n\t\t"
    for i in range(0, data.size()):
        header_contents += f"{data[i]}"
        if i != data.size() - 1:
            header_contents += ", "

    header_contents += "\t}\n"
    """

    header_contents += "};\n}\n\n"
    header_contents += f"#endif //{header_include_guard}\n"

    headerfile.write(header_contents)
    headerfile.close()

export_path = sys.argv[1]
arguments = sys.argv[2]
for resource_path in arguments.split(" "):
    print(f"ResourceEmbedder.py : creating header for '{resource_path}'")
    create_header(resource_path, export_path)
exit(0)
