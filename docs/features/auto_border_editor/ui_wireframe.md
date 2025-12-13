# Auto Border Editor: UI Wireframe

This document visualizes the layout of the **Auto Border Editor** (650x520 px).

## 1. Main Layout Structure

The dialog consists of a common header and a tabbed interface.

```text
+-------------------------------------------------------------+
|  Auto Border Editor                                     [X] |
+-------------------------------------------------------------+
|                                                             |
|  +-- Common Properties ----------------------------------+  |
|  |  Name: [_______________________]   ID: [ 1  ] [-] [+] |  |
|  +-------------------------------------------------------+  |
|                                                             |
|  [ Border ] [ Ground ] [ Wall ]                             |
| +---------------------------------------------------------+ |
| |                                                         | |
| |  < Active Tab Content Area >                            | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| |                                                         | |
| +---------------------------------------------------------+ |
|                                                             |
+-------------------------------------------------------------+
```

## 2. Tab: Border Editor (Default View)

This tab is used for defining the 3x3 tile patterns.

```text
| [ Border ] [ Ground ] [ Wall ]                             |
+---------------------------------------------------------+ |
| +-- Border Properties -------------------------------+  | |
| | Group: [ 0 ]     [ ] Optional   [ ] Ground         |  | |
| |                                                    |  | |
| | Load Existing: [ Select Border v ]                 |  | |
| +----------------------------------------------------+  | |
|                                                         | |
| +-- Border Grid ----------+  +-- Preview -------------+ | |
| |                         |  |                        | | |
| |  +---+---+---+          |  |  [ Rendered Preview ]  | | |
| |  | N | N | N |          |  |                        | | |
| |  +---+---+---+          |  |                        | | |
| |  | W | C | E |          |  |                        | | |
| |  +---+---+---+          |  |                        | | |
| |  | S | S | S |          |  |                        | | |
| |  +---+---+---+          |  |                        | | |
| |                         |  |                        | | |
| | ID: [ 0 ] [Browse] [Add]|  |                        | | |
| |                         |  |                        | | |
| +-------------------------+  +------------------------+ | |
|                                                         | |
| [ Clear ] [ Save Border ]                  [ Close ]    | |
+---------------------------------------------------------+ |
```

## 3. Tab: Ground Brush

This tab links borders to ground tiles and creates brushes.

```text
| [ Border ] [ Ground ] [ Wall ]                             |
+---------------------------------------------------------+ |
| +-- Ground Brush Properties ---------------------------+  | |
| | Tileset: [ Nature       v ]   Server Look ID: [ 0 ]  |  | |
| | Z-Order: [ 0 ]                                       |  | |
| | Load Existing: [ Select Ground Brush v ]             |  | |
| +----------------------------------------------------+  | |
|                                                         | |
| +-- Ground Items -----------------------------------+   | |
| | [ List of Item IDs and Chances                  ] |   | |
| | [ ...                                           ] |   | |
| | [ ...                                           ] |   | |
| |                                                   |   | |
| | Item ID: [ 0 ]  Chance: [ 10 ]  [Browse] [Add]    |   | |
| +---------------------------------------------------+   | |
|                                                         | |
| +-- Border for Ground Brush ------------------------+   | |
| | Alignment: [ Outer v ]   [x] To None  [ ] Inner   |   | |
| | Border ID: (Uses Common ID)                       |   | |
| +---------------------------------------------------+   | |
|                                                         | |
| [ Clear ] [ Save Ground ]                  [ Close ]    | |
+---------------------------------------------------------+ |
```

## 4. Tab: Wall Brush

This tab manages wall brushes.

```text
| [ Border ] [ Ground ] [ Wall ]                             |
+---------------------------------------------------------+ |
| +-- Existing Brushes -------------------------------+   | |
| | [ Select Wall Brush v ]                           |   | |
| +---------------------------------------------------+   | |
|                                                         | |
| +-- Wall Properties --------------------------------+   | |
| | Server Look ID: [ 0 ]                             |   | |
| | [ ] Optional    [ ] Ground                        |   | |
| +---------------------------------------------------+   | |
|                                                         | |
| +-- Preview ----------------------------------------+   | |
| |                                                   |   | |
| |          [ Wall Brush Visual Preview ]            |   | |
| |                                                   |   | |
| +---------------------------------------------------+   | |
|                                                         | |
| [ Clear All ]               [ Save Wall ]  [ Close ]    | |
+---------------------------------------------------------+ |
```
