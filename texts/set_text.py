import json
import codecs
import sys


if len(sys.argv) < 3:
    print 'set_text.py <source_json> <merge_texts>'
    exit(0)
    
f = codecs.open(sys.argv[1], encoding='utf-8')

script_data = json.load(f)
objects_keys = script_data.keys()

texts_file = codecs.open(sys.argv[2], mode='r', encoding='utf-8')

if u'scenes' not in script_data:
    print 'invalid json script'
    exit()

texts = []


def parse_texts_file():
    while True:
        magic = texts_file.readline()
        
        if not magic:
            break

        if magic[0] != '@':
            continue

        check_text = texts_file.readline()
        cn_text = texts_file.readline()
        nul_line = texts_file.readline()
        if not check_text or not cn_text or not nul_line:
            break
        
        str_id = int(magic.replace('\n', '').replace('@', ''))
        check_text = check_text.replace('\n', '').replace('\\n', '\n')
        cn_text = cn_text.replace('\n', '').replace('\\n', '\n')
        str_rep = {'id': str_id, 'check_text': check_text, 'cn_text': cn_text}
        texts.append(str_rep)


def get_text(text_id):
    for text in texts:
        if text['id'] == text_id:
            return text['cn_text']
    return None


def replace_text_to_json():
    scenes = script_data['scenes']
    index = 0

    for i in scenes:
        if 'texts' in i:
            j_texts = i['texts']
            for j in j_texts:
                for x in range(0, 3):
                    if j[x] is not None and j[x] != '':
                        new_text = get_text(index)
                        if not new_text:
                            continue
                        j[x] = new_text
                        index += 1


parse_texts_file()
replace_text_to_json()

f.close()

complete_file = codecs.open(sys.argv[1], mode='w', encoding='utf-8')
complete_file.writelines(json.dumps(script_data, ensure_ascii=False, indent=4, sort_keys=False))
complete_file.flush()
complete_file.close()
