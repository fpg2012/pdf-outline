# pdf-outline

pdf-outline is a tool to extract and replace outline from PDF file.

## usage

extract outline from pdf file

```
./pdf-outline <pdf file> -x <outline json file>
```

replace outline with given json

```
./pdf-outline <pdf file> -r <outline json file> -o <output pdf file>
```

## build

Install dependencies: `icu4c`, `zlib`, `nlohmann_json`

Clone this repository.

```bash
git clone https://github.com/fpg2012/pdf-outline.git
```

Inside the `pdf-outline` directory, clone `pindf`, a low-level pdf parser library.

```bash
cd pdf-outline
git clone https://github.com/fpg2012/pindf.git
```

Build with `make`

```bash
make
make test
```

Create a soft-link to libpindf

```bash
ln -s pindf/libpindf.dylib libpindf.dylib # mac
ln -s pindf/libpindf.so libpindf.so # linux
ln -s pindf/libpindf.dll libpindf.dll # windows
```

## example JSON format

```json
{
    "chd": [
        {
            "title": "前言",
            "destination": {
                "page": 5
            }
        },
        {
            "title": "作者题词",
            "destination": {
                "page": 10
            }
        },
        {
            "title": "目录",
            "destination": {
                "page": 11
            }
        },
        {
            "title": "牡丹亭",
            "destination": {
                "page": 14
            },
            "chd": [
                {
                    "title": "第一齣 標目",
                    "page": 14
                },
                {
                    "title": "第二齣 言懷",
                    "page": 17
                },
                {
                    "title": "第三齣 訓女",
                    "page": 21
                },
            ]
        },
        {
            "title": "附录",
            "page": 323,
            "chd": [
                {
                    "title": "附录一 关于版本的说明",
                    "page": 323
                },
                {
                    "title": "附录二 杜丽娘慕色还魂画本",
                    "page": 326
                }
            ]
        }
    ]
}
```

basic structure

```
{
    "title": "helloworld", // title of the section
    "destination": {
        "dest_arr": "[ 34 0 R /FitH ]", // original dest object
        "page": 23 // page index, starts from 1
    },
    "chd": [
        // ... subsections
    ]
}
```

simplified structure

```
{
    "title": "helloworld", // title of the section
    "page": 23, // page index, starts from 1
    "chd": [
        // ... subsections
    ]
}
```