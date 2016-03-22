import json
import codecs

f = file(r'01_com_027_01.ks.psb.json')

script_data = json.load(f)
objects_keys = script_data.keys()

texts_file = codecs.open('01_com_027_01.txt', mode='r', encoding='utf-8')

if u'scenes' not in script_data:
    print 'invalid json script'
    exit()

texts = []


def parse_texts_file():
    while True:
        tag = texts_file.readline()
        check_text = texts_file.readline()
        cn_text = texts_file.readline()
        nul_line = texts_file.readline()

        if not nul_line or not tag or not check_text or not cn_text:
            break

        str_id = int(tag.replace('\n', '').replace('@', ''))
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
                        j[x] = get_text(index)
                        index += 1


parse_texts_file()
replace_text_to_json()

complete_file = codecs.open('01_com_027_01.new.json', mode='w', encoding='utf-8')
complete_file.writelines(json.dumps(script_data, ensure_ascii=False, indent=4, sort_keys=False))
complete_file.flush()
complete_file.close()
