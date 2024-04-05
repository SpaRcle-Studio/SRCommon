from Common import *

Config.set_library_path(os.path.dirname(os.path.realpath(__file__)))

parser = argparse.ArgumentParser(
    prog='Codegen',
    description='This program creates a header file with the binary content of a file')
parser.add_argument('--project-directory')
parser.add_argument('--config-file')
parser.add_argument('--export-directory')

print("SRCodegen.py : parsing arguments")

args = parser.parse_args()

print(f'SRCodegen project directory: {args.project_directory}')
print(f'SRCodegen config file: {args.config_file}')
print(f'SRCodegen export directory: {args.export_directory}')

if not os.path.isfile(args.config_file):
    print(f"Config file does not exist: {args.config_file}")
    exit(-1)

codegen_config_xml = ET.parse(args.config_file)
codegen_config_root = codegen_config_xml.getroot()

codegen_files = []

try:
    for include_directory_node in codegen_config_root.findall('IncludeDirectory'):
        include_directory = os.path.normpath(os.path.join(args.project_directory, include_directory_node.get('Path')))
        codegen_files.append(include_directory)
except:
    pass

try:
    for file_node in codegen_config_root.findall('File'):
        file_path = os.path.normpath(os.path.join(args.project_directory, file_node.get('Path')))
        codegen_files.append(file_path)
except:
    pass

for codegen_file in codegen_files:
    print(f"File: {codegen_file}")

    cpp_file = os.path.join(args.export_directory, 'codegen_parse.cpp')

    os.makedirs(os.path.dirname(cpp_file), exist_ok=True)
    #shutil.copyfile(codegen_file, cpp_file)

    index = clang.cindex.Index.create()
    translation_unit = index.parse(cpp_file)

    def filter_node_list_by_node_kind(
            nodes: typing.Iterable[clang.cindex.Cursor],
            kinds: list
    ) -> typing.Iterable[clang.cindex.Cursor]:
        result = []

        for i in nodes:
            if i.kind in kinds:
                result.append(i)

        return result

    all_classes = filter_node_list_by_node_kind(translation_unit.cursor.get_children(), [clang.cindex.CursorKind.CLASS_DECL, clang.cindex.CursorKind.STRUCT_DECL])

    for i in all_classes:
        print (i.spelling)

    print("Finished parsing file")

exit(0)
