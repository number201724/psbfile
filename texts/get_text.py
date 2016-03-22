import json
import codecs

f = file(r'01_com_027_01.ks.psb.json')

script_data = json.load(f)
objects_keys = script_data.keys()

texts_file = codecs.open('01_com_027_01.txt', mode='w', encoding='utf-8')

if u'scenes' not in script_data:
    print 'invalid json script'
    exit()

scenes = script_data['scenes']
index = 0

for i in scenes:
    if 'texts' in i:
        texts = i['texts']
        for j in texts:
            for x in range(0, 3):
                if j[x] is not None and j[x] != '':
                    texts_file.writelines('@' + str(index) + '\n')
                    index += 1
                    texts_file.writelines('//' + j[x].replace('\n', '\\n') + '\n')
                    texts_file.writelines(j[x].replace('\n', '\\n') + '\n')
                    texts_file.writelines('\n')