# pdf-outline

pdf-outline is a tool to extract and replace outline from PDF file.

## build

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