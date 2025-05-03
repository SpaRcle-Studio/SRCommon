import os
import numpy as np
import argparse
import hashlib
import zlib

print("ResourceEmbedder.py: running...")
def needs_update(path, export_path):
    if not os.path.exists(path):
        print(f"Path does not exist: {path}")
        return

    if not os.path.isfile(path):
        print(f"Path is not a file: {path}")
        return


    filename, file_extension = os.path.splitext(os.path.basename(path))
    file_extension = file_extension[1:]

    header_name = f"{filename}{file_extension}"
    header_path = f"{export_path}/EmbedResources/{header_name}.h"
    if not os.path.exists(header_path):
        print("ResourceEmbedder.py: header does not exist, creating a new one.")
        return True

    hash_path = f"{export_path}/EmbedResources/Hashes/{header_name}.hash"
    if os.path.exists(hash_path):
        current_hash = hashlib.md5(open(path, "rb").read()).hexdigest()
        previous_hash = open(hash_path, "r").read()
        if current_hash != previous_hash:
            print(f"ResourceEmbedder.py: hashes are not equal, creating new header: '{current_hash}' != '{previous_hash}'.")
            return True

    return False


def create_cxx(path):
    if not os.path.exists(path):
        raise FileNotFoundError(f"ResourceEmbedder.py: path does not exist: {path}")

    print(f"ResourceEmbedder.py: creating cxx at '{path}'.")
    files = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))]

    if os.path.isfile(os.path.join(path, "EmbedResources.cxx")):
        os.remove(f"{path}/EmbedResources.cxx")

    cxx_file = open(f"{path}/EmbedResources.cxx", "w")
    cxx_file.write("/// This file is created by ResourceEmbedder.py\n\n")
    cxx_file.write("#include <Utils/stdInclude.h>\n\n")
    for file in files:
        if file.endswith(".h"):
            cxx_file.write(f"#include \"{file}\"\n")
    cxx_file.close()

def bytes_to_c_arr(data):
    return [format(b, '#04x') for b in data]


def read_file(file_name):
    data = np.fromfile(file_name, dtype='uint8')
    data = bytearray(data)
    return data

def compress_byte_data_and_convert(byte_data) -> str:
    compressed_data = zlib.compress(byte_data)  # Сжимаем данные с помощью zlib
    hex_string = compressed_data.hex()  # Преобразуем сжатые данные в строку hex
    return hex_string

def create_array(name, data, path):
    print(f'Working directory: {working_directory}, Path: {path}')
    path = path.split(working_directory + '/')
    print(f'Path: {path}')
    path = path[1]

    encoded = compress_byte_data_and_convert(data)
    if len(encoded) == 0:
        raise ValueError("ResourceEmbedder.py: encoded data is empty")

    max_line_length = 512

    compressed_data = ''
    # write all data as string, max max_line_length symbols per line
    for i in range(0, len(encoded)):
        if i % max_line_length == 0:
            compressed_data += "\t\t\t\""
        compressed_data += encoded[i]
        if i % max_line_length == (max_line_length - 1) or i == len(encoded) - 1:
            if i == len(encoded) - 1:
                compressed_data += "\";\n"
            else:
                compressed_data += "\"\n"
        else:
            compressed_data += ""

    content = (
        f"\t\t/// Encoded as zlib + hex\n\n"
        f"\t\tconstexpr static const std::string_view path = \"{path}\";\n"
        f"\t\tconstexpr static const uint64_t decompressedSize = {len(data)};\n"
        f"\t\tconstexpr static const uint64_t compressedSize = {len(encoded)};\n"
        f"\t\tconstexpr static const char* data = \n{compressed_data}"
    )

    return content


def create_header(path, export_path):
    #print(f"ResourceEmbedder.py : creating header for '{path}'")
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
    class_name = header_name.replace("-", "_")
    header_include_guard = "CODEGEN_" + header_name.upper() + "_H"
    header_include_guard = header_include_guard.replace("-", "_")

    if not os.path.exists(f"{export_path}/EmbedResources"):
        os.mkdir(f"{export_path}/EmbedResources")

    headerfile = open(f"{export_path}/EmbedResources/{header_name}.h", "w", encoding="utf-8")

    header_contents = ""
    header_contents += "/// This file is created by ResourceEmbedder.py\n\n"
    header_contents += f"#ifndef {header_include_guard}\n"
    header_contents += f"#define {header_include_guard}\n\n"
    header_contents += "#include <Utils/Resources/ResourceEmbedder.h>\n\n"
    header_contents += "#include <Utils/Common/Base64.h>\n\n"
    header_contents += "namespace ResourceEmbedder::Resources {\n"
    header_contents += f"\tclass {class_name} " + " {\n"
    header_contents += "\tpublic:\n"
    header_contents += f"\t\t{class_name}() = delete;\n\n"

    header_contents += create_array(header_name, read_file(path), path)

    header_contents += "\n\tprivate:\n"
    header_contents += (f"\t\tSR_MAYBE_UNUSED SR_INLINE static bool codegenRegister = "
                        f"SR_UTILS_NS::ResourceEmbedder::Instance().RegisterResource<{class_name}>();\n\n")
    header_contents += "\t};\n}\n\n"
    header_contents += f"#endif //{header_include_guard}\n"

    headerfile.write(header_contents)
    headerfile.close()

    hash_path = f"{export_path}/EmbedResources/Hashes/{header_name}.hash"
    if not os.path.exists(f"{export_path}/EmbedResources/Hashes"):
        os.mkdir(f"{export_path}/EmbedResources/Hashes")

    hash_file = open(hash_path, "w")
    hash_file.write(hashlib.md5(open(path, "rb").read()).hexdigest())
    hash_file.close()

parser = argparse.ArgumentParser(
                    prog='ResourceEmbedder',
                    description='This program creates a header file with the binary content of a file')
parser.add_argument('--export-directory', help='The path where the header file will be exported')
parser.add_argument('--working-directory', help='The working directory')
parser.add_argument('--resources', help='The resources to embed')

args = parser.parse_args()
working_directory = args.working_directory

if working_directory == "" and args.export_directory == "":
    print("ResourceEmbedder.py: working directory and export directory are not set.")
    exit(0)

print(f"ResourceEmbedder.py: counting resources: {len(args.resources.split('|'))}")

resources = filter(None, args.resources.split('|'))

cxx_needs_update = False

embed_resources_path = f'{args.export_directory}/EmbedResources/EmbedResources.cxx'

if not os.path.isfile(embed_resources_path):
    cxx_needs_update = True
    print(f"ResourceEmbedder.py: cxx does not exist by path '{embed_resources_path}'. creating a new one.")

for resource_path in resources:
    resource_path = resource_path.replace("\\", "/")
    if os.path.isdir(resource_path):
        for filename in os.listdir(resource_path):
            file_path = os.path.join(resource_path, filename)
            if os.path.isfile(file_path) and needs_update(file_path, args.export_directory):
                create_header(file_path, args.export_directory)
                cxx_needs_update = True
                print(f"ResourceEmbedder.py: {file_path} needs update.")
    elif needs_update(resource_path, args.export_directory):
        create_header(resource_path, args.export_directory)
        cxx_needs_update = True
        print(f"ResourceEmbedder.py: {resource_path} needs update.")

if cxx_needs_update:
    create_cxx(f"{args.export_directory}/EmbedResources")
else:
    print("ResourceEmbedder.py: no updates needed.")

print("ResourceEmbedder.py: finished.")

exit(0)
