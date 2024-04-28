from Common import *

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
        print("ResourceEmbedder.py : header does not exist, creating a new one.")
        return True

    hash_path = f"{export_path}/EmbedResources/Hashes/{header_name}.hash"
    if os.path.exists(hash_path):
        current_hash = hashlib.md5(open(path,"rb").read()).hexdigest()
        previous_hash = open(hash_path, "r").read()
        if current_hash != previous_hash:
            print(f"ResourceEmbedder.py : hashes are not equal, creating new header: "
                  f"'{hashlib.md5(open(path, "rb").read()).hexdigest()}' != '{open(hash_path, "r").read()}'.")
            return True

    return False

def create_cxx(path):
    print(f"ResourceEmbedder.py : creating cxx at '{path}'.")
    files = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))]

    if os.path.exists(path):
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


def create_array(name, data, path):
    size = str(len(data))
    path = path.split(working_directory + '/')[1]
    static_content = (f"\t\tconstexpr static const uint64_t size = {size};"
                      f"\n\t\tconstexpr static const char path[] = \"{path}\";"
                      f"\n\t\tconstexpr static const unsigned char data[") + size + "] = "
    array_content = "{\n\t\t\t"
    byte_array = bytes_to_c_arr(data)
    for i in range(0, len(byte_array)):
        if i == len(byte_array) - 1:
            array_content += f"{byte_array[i]}" + '\n\t\t};\n'
        else:
            array_content += f"{byte_array[i]}, "
        if (i + 1) % 18 == 0:
            array_content += "\n\t\t\t"


    #array_content = '\n' + array_content
    final_content = static_content + array_content
    return final_content


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

    headerfile = open(f"{export_path}/EmbedResources/{header_name}.h", "w")

    header_contents = ""
    header_contents += "/// This file is created by ResourceEmbedder.py\n\n"
    header_contents += f"#ifndef {header_include_guard}\n"
    header_contents += f"#define {header_include_guard}\n\n"
    header_contents += "#include <Utils/Resources/ResourceEmbedder.h>\n\n"
    header_contents += "namespace ResourceEmbedder::Resources {\n"
    header_contents += f"\tclass {class_name} " + " {\n"
    header_contents += "\tpublic:\n"
    header_contents += f"\t\t{class_name}() = delete;\n"

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
    exit(0)

resources = filter(None, args.resources.split('|'))
cxx_needs_update = False
for resource_path in resources:
    resource_path = resource_path.replace("\\", "/")
    if os.path.isdir(resource_path):
        for filename in os.listdir(resource_path):
            file_path = os.path.join(resource_path, filename)
            if os.path.isfile(file_path) and needs_update(file_path, args.export_directory):
                create_header(file_path, args.export_directory)
                cxx_needs_update = True
    elif needs_update(resource_path, args.export_directory):
        create_header(resource_path, args.export_directory)
        cxx_needs_update = True

if cxx_needs_update:
    create_cxx(f"{args.export_directory}/EmbedResources")
exit(0)
