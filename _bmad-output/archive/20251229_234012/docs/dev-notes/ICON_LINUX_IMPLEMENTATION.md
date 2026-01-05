# Implementação de Ícone para Linux

## Visão Geral

Esta implementação adiciona suporte para ícones em executáveis Linux compilados. No Linux, diferentemente do Windows, não há suporte nativo para incorporar ícones diretamente em executáveis ELF. A solução implementada usa uma abordagem híbrida:

1. **Conversão do ícone ICO para PNG** em múltiplos tamanhos (16x16 até 256x256)
2. **Criação de arquivo .desktop** para integração com ambientes desktop Linux
3. **Tentativa de incorporação via objcopy** (experimental, pode não funcionar em todos os ambientes)

## Como Funciona

### 1. Conversão de Ícone

O sistema detecta automaticamente:
- Se o ImageMagick `convert` está disponível
- Se o arquivo `rme_icon.ico` existe na raiz do projeto

Se ambas condições forem verdadeiras, o ícone é convertido para PNG em múltiplos tamanhos durante a compilação:
- 16x16, 32x32, 48x48, 64x64, 128x128, 256x256

Os arquivos PNG são criados em `${CMAKE_BINARY_DIR}/icons/`

### 2. Arquivo .desktop

Um arquivo `.desktop` é criado automaticamente em `${CMAKE_BINARY_DIR}/canary-map-editor.desktop`. Este arquivo:
- Define metadados da aplicação (nome, descrição, categoria)
- Aponta para o executável
- Especifica o ícone a ser usado

**Para usar o arquivo .desktop:**
```bash
# Copiar para ~/.local/share/applications/ (usuário) ou /usr/share/applications/ (sistema)
cp build/canary-map-editor.desktop ~/.local/share/applications/
# Atualizar cache do desktop
update-desktop-database ~/.local/share/applications/
```

### 3. Incorporação via objcopy (Experimental)

O sistema tenta incorporar o ícone diretamente no executável usando `objcopy` para adicionar uma seção `.icon`. Esta abordagem:
- Pode funcionar com alguns gerenciadores de arquivos
- Não é garantida para todos os ambientes desktop
- É uma tentativa experimental

## Estrutura de Arquivos

Após a compilação, você encontrará:
```
build/
├── icons/
│   ├── rme_icon_16.png
│   ├── rme_icon_32.png
│   ├── rme_icon_48.png
│   ├── rme_icon_64.png
│   ├── rme_icon_128.png
│   └── rme_icon_256.png
└── canary-map-editor.desktop
```

## Dependências

- **ImageMagick** (`convert`): Necessário para conversão ICO → PNG
- **objcopy** (opcional): Para tentativa de incorporação direta

## Notas Importantes

1. **Formato ICO**: O arquivo `rme_icon.ico` deve estar na raiz do projeto
2. **Visualização no Ubuntu**: O ícone aparecerá corretamente quando:
   - O arquivo `.desktop` estiver instalado
   - O ícone PNG estiver acessível no caminho especificado
3. **Diferença do Windows**: No Windows, o ícone é incorporado via arquivo `.rc` (resource file). No Linux, a abordagem padrão é usar arquivos `.desktop`

## Troubleshooting

### O ícone não aparece no Ubuntu
1. Verifique se o arquivo `.desktop` foi criado em `build/`
2. Instale o arquivo `.desktop` em `~/.local/share/applications/`
3. Execute `update-desktop-database ~/.local/share/applications/`
4. Verifique se os arquivos PNG foram criados em `build/icons/`

### Erro durante compilação
- Verifique se o ImageMagick está instalado: `which convert`
- Verifique se `rme_icon.ico` existe na raiz do projeto

## Referências

- [Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html)
- [Linux Executable Icons](https://wiki.archlinux.org/title/Desktop_entries)
