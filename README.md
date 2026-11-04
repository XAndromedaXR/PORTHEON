# PORTHEON — Ports & Mods

**PORTHEON** is a launcher for the PC ports of **EA SPORTS UFC 1, 2, 3, 4 (PS4), UFC 5 and UFC 6 (PS5)**, created and maintained by **Marcelo Miyake (XAndromedaXR)**. It finds the game, reads your PC and picks the best settings for each UFC by itself.

> **PORTHEON is free. I do not sell copies of PORTHEON — I only accept donations** from anyone who wants to support the project.
>
> **No game files are included.** You need your own copy of each game.
> This project is not affiliated with or endorsed by Electronic Arts, Sony Interactive Entertainment or the UFC. *EA SPORTS UFC* and *PlayStation* are trademarks of their owners and are named only to identify the games.

💬 **Discord:** https://discord.gg/FEQ752rUz

*(Versão em português logo abaixo.)*

---

## What's new in 4.3.1

- **UFC 5 without slow motion:** UFC 5 moved its world one fixed step per drawn frame, made for 60 FPS. PORTHEON now changes the game's clock at start, so every frame moves by the real time that passed: fights move at the PS5's pace at any FPS, with no lag, like UFC 6. The engine also runs at High priority.
- **UFC 5 much lighter on 1080p and 1440p screens:** the engine told the game it was on a 4K TV, so UFC 5 drew every frame for 4K. Now the game is told the size of your screen, and below 4K it renders its 1080p mode, like a PS5 on a Full HD TV: on a Ryzen 5 5500 + RTX 4060 the fighter select went from 12-20 to 39-53 FPS and the fight from 24-31 to 31-45, with less video memory used. 4K screens keep the 4K mode.
- **No black band in UFC 5's fighter and kit selection:** the game's FSR2 is on again by default; the plain stretch that replaced it left the right tenth of the screen black when the game lowered its resolution.
- **UFC 5 and UFC 6 start ready to play:** a first start gets a save past the opening tutorial, on any PC, and the shaders already built for your graphics card when PORTHEON has them, so the first session no longer stutters while thousands of shaders are built. Each card model compiles its own shaders, so the caches come per card (`games\UFC5\cache-inicial` and `games\UFC6\cache-inicial`, one folder per card): this release has the NVIDIA RTX 4060, and more cards come in as players send theirs (see `LEIA-ME.txt` there). The processor does not matter. A save or cache you already have is never touched.
- **UFC 6 without slow motion (4.3):** fights run at the PS5's pace, 60 steps a second apart from the screen's FPS, and with more FPS.
- **UFC 6 on 5 GB cards and 8 GB of RAM:** 5 GB cards use the 4 GB memory profile, and if UFC 6 closes for lack of RAM, PORTHEON offers to set up the page file, as it does for UFC 1-4.
- **Parsec in UFC 5 and UFC 6:** Parsec mode (Online section) keeps the controller working with Parsec's window in front, in Mailbox at 60 Hz.
- **UFC 3 and UFC 4 on the new engine on every card:** in UFC 4, 11% more FPS and fewer stutters, with the fixes for codes 20 and 23 and for the Career. If the game closes right at the start with it, PORTHEON goes back to the port's own engine by itself.
- **The audio watchdog thread** of UFC 5 and UFC 6, which kept a whole processor core busy, is off.
- UFC 1 to 4 start in PORTHEON's language the first time.
- **No more restarting by itself:** when a game you left in the background closes, PORTHEON no longer jumps in front of the program you are using, so a key you type there can no longer start the game again.
- **A clearer driver message:** if the graphics driver lacks a Vulkan extension the engine needs, PORTHEON names it and asks for a driver update, instead of a generic error.

## Games

| Game | Platform | What you need | Port |
|---|---|---|---|
| EA SPORTS UFC (2014) | PS4 | `CUSA00222`, update 1.08 | native PC port |
| EA SPORTS UFC 2 | PS4 | `CUSA01968`, update 1.13 | native PC port |
| EA SPORTS UFC 3 | PS4 | `CUSA06534`, update 1.14 | native PC port |
| EA SPORTS UFC 4 | PS4 | `CUSA14209` or `CUSA14204`, update 28.00 | native PC port |
| EA SPORTS UFC 5 | PS5 | `PPSA03541`, version 01.031 | PC port (KytyPS5 engine) |
| EA SPORTS UFC 6 | PS5 | `PPSA23566`, version 01.004 | PC port (KytyPS5 engine) |

## How to install

1. Download `PORTHEON-x.x.x-Windows-x64.zip` from the [latest release](../../releases/latest).
2. Extract it into a **new, empty folder** (never over an older version, and not inside OneDrive).
3. Open `PORTHEON.exe`. On Linux / Steam Deck, run `portheon.sh` (experimental).
4. PORTHEON looks for your games on the PC. If it does not find one, open the game, choose **Game files** and pick the folder of your extracted game.
5. Click **PLAY**.

**Requirements:** Windows 10 (1903 or newer) or 11, 64-bit; a graphics card with Vulkan 1.3 (NVIDIA, AMD or Intel Arc) and an up-to-date driver. UFC 1-4: an Intel Core 4th generation (Haswell) or AMD Ryzen processor or newer (UFC 3 also runs on Intel Core 2nd/3rd generation through PORTHEON's compatibility mode). UFC 5 and 6: a processor with AVX2, 16 GB of RAM and, for UFC 6, 8 GB of video memory recommended (profiles for 4 and 6 GB cards).

## What PORTHEON does

- One window for every UFC: video, gameplay and controller settings written straight into each port.
- Automatic setup for your PC (processor, graphics card, video memory, RAM and monitor), redone when the PC changes.
- Fixes known crashes by itself: after a graphics card stop (code 23) or a game fault (code 3) it changes what caused it for the next PLAY and explains it in words.
- Controllers: PS5, PS4, PS3, Xbox and generic; two controllers are two players in UFC 1-4; Parsec mode for playing with a friend.
- Profiles, automatic save backups, cheats (UFC 4), desktop shortcuts, 16 languages.

## Credits

- **PORTHEON:** Marcelo Miyake (**XAndromedaXR**) — the launcher, its interface, and the fixes and optimizations of every port.
- **UFC 5 engine:** the KytyPS5 build by **Cozmaa** and **Brotski**. **XAndromedaXR improved their build** for PORTHEON: the game's speed set from the real FPS, an AMD build that survives driver faults, crowd/shadows/seats switches and the audio watchdog off.
- **UFC 6 engine:** the KytyPS5 build by **Peepo**. **XAndromedaXR improved his build** for PORTHEON: fights at the PS5's pace, the audio watchdog off, video memory profiles for 4, 6 and 8 GB cards and a guard against video memory crashes.
- **FPS patch:** **lepexa_cell** (Lepexa). **XAndromedaXR improved his FPS patch** for PORTHEON. The other community patches: Lepexa and Antigravity.
- **UFC 1-4 ports:** built on **GNS** (GNSB)'s UFC 3 PS4 port, on SacredFire / bbport and on shadPS4.
- Also: KytyPS5 and Kyty (InoriRus), roshanarunk, Senaxx and EmK530 (UFC 5 engine work), deanzx06 (Sandy/Ivy Bridge compatibility), the GoldHEN Cheat Repository (UFC 4 cheats), SDL_GameControllerDB.
- And also credits to ken9l

## License

PORTHEON and the ports are free software under the GNU GPL, version 2 or later. The source code is published with each release; the `licenses` folder of the package has every license and the credits.

---

# PORTHEON — Ports & Mods (português)

O **PORTHEON** é um launcher dos ports para PC do **EA SPORTS UFC 1, 2, 3, 4 (PS4), UFC 5 e UFC 6 (PS5)**, criado e mantido por **Marcelo Miyake (XAndromedaXR)**. Ele acha o jogo, lê o seu PC e escolhe sozinho as melhores configurações de cada UFC.

> **O PORTHEON é gratuito. Eu não vendo cópias do PORTHEON — aceito apenas doações** de quem quiser apoiar o projeto.
>
> **Nenhum arquivo dos jogos vem junto.** Você precisa da sua própria cópia de cada jogo.
> Este projeto não é afiliado nem endossado pela Electronic Arts, pela Sony Interactive Entertainment ou pelo UFC. *EA SPORTS UFC* e *PlayStation* são marcas dos seus donos e aparecem só para identificar os jogos.

💬 **Discord:** https://discord.gg/FEQ752rUz

## Novidades da 4.3.1

- **UFC 5 sem câmera lenta:** o UFC 5 andava um passo fixo a cada quadro desenhado, feito para 60 FPS. Agora o PORTHEON muda o relógio do jogo ao abrir, e cada quadro avança o tempo real que passou: a luta anda na velocidade do PS5 com qualquer FPS, sem atraso, como no UFC 6. O motor também roda com prioridade Alta.
- **UFC 5 bem mais leve em telas 1080p e 1440p:** o motor dizia ao jogo que ele estava numa TV 4K, e o UFC 5 desenhava cada quadro para 4K. Agora o jogo recebe o tamanho da sua tela e, abaixo de 4K, renderiza no modo 1080p, como um PS5 numa TV Full HD: num Ryzen 5 5500 + RTX 4060 a seleção de lutador foi de 12-20 para 39-53 FPS e a luta de 24-31 para 31-45, com menos memória de vídeo. Telas 4K continuam no modo 4K.
- **Sem faixa preta na seleção de lutador e de kit do UFC 5:** o FSR2 do jogo voltou ligado por padrão; o esticamento simples que o substituía deixava preto o décimo da direita da tela quando o jogo baixava a resolução.
- **UFC 5 e UFC 6 abrem prontos para jogar:** na primeira vez entram um save que já passou do tutorial de início, em qualquer PC, e os shaders já montados para a sua placa de vídeo quando o PORTHEON tem os dela, então a primeira sessão não engasga mais montando milhares de shaders. Cada modelo de placa compila os próprios shaders, por isso os caches vêm por placa (`games\UFC5\cache-inicial` e `games\UFC6\cache-inicial`, uma pasta por placa): esta versão traz a NVIDIA RTX 4060, e outras placas entram conforme os jogadores mandam os seus (veja o `LEIA-ME.txt` lá). O processador não importa. Um save ou cache que você já tem nunca é tocado.
- **UFC 6 sem câmera lenta (4.3):** a luta roda na velocidade do PS5, a 60 passos por segundo, separados do FPS da tela, e com mais FPS.
- **UFC 6 em placas de 5 GB e com 8 GB de RAM:** placas de 5 GB usam o perfil de memória de 4 GB e, se o UFC 6 fechar por falta de RAM, o PORTHEON oferece configurar o arquivo de paginação, como já fazia no UFC 1 a 4.
- **Parsec no UFC 5 e no UFC 6:** o Modo Parsec (aba Online) mantém o controle funcionando com a janela do Parsec na frente, em Mailbox a 60 Hz.
- **UFC 3 e UFC 4 no motor novo em todas as placas:** no UFC 4, 11% mais FPS e menos travadinhas, com as correções dos códigos 20 e 23 e da Carreira. Se o jogo fechar logo no começo com ele, o PORTHEON volta sozinho para o motor do port.
- **A thread vigia de áudio** do UFC 5 e do UFC 6, que ocupava um núcleo inteiro do processador, foi desligada.
- UFC 1 a 4 abrem no idioma do PORTHEON na primeira vez.
- **O jogo não reabre mais sozinho:** quando um jogo que você deixou em segundo plano fecha, o PORTHEON não pula mais na frente do programa que você está usando, então uma tecla digitada lá não abre o jogo de novo.
- **Mensagem mais clara do driver:** se o driver de vídeo não tiver uma extensão Vulkan que o motor precisa, o PORTHEON diz qual é e pede para atualizar o driver, em vez de um erro genérico.

## Jogos

| Jogo | Plataforma | O que você precisa | Port |
|---|---|---|---|
| EA SPORTS UFC (2014) | PS4 | `CUSA00222`, update 1.08 | port nativo para PC |
| EA SPORTS UFC 2 | PS4 | `CUSA01968`, update 1.13 | port nativo para PC |
| EA SPORTS UFC 3 | PS4 | `CUSA06534`, update 1.14 | port nativo para PC |
| EA SPORTS UFC 4 | PS4 | `CUSA14209` ou `CUSA14204`, update 28.00 | port nativo para PC |
| EA SPORTS UFC 5 | PS5 | `PPSA03541`, versão 01.031 | port para PC (motor KytyPS5) |
| EA SPORTS UFC 6 | PS5 | `PPSA23566`, versão 01.004 | port para PC (motor KytyPS5) |

## Como instalar

1. Baixe o `PORTHEON-x.x.x-Windows-x64.zip` da [versão mais recente](../../releases/latest).
2. Extraia numa **pasta nova e vazia** (nunca por cima de uma versão antiga, e fora do OneDrive).
3. Abra o `PORTHEON.exe`. No Linux / Steam Deck, rode o `portheon.sh` (experimental).
4. O PORTHEON procura os seus jogos no PC. Se não achar algum, abra o jogo, escolha **Arquivos do jogo** e indique a pasta do seu jogo extraído.
5. Clique em **JOGAR**.

**Requisitos:** Windows 10 (1903 ou mais novo) ou 11, 64 bits; placa de vídeo com Vulkan 1.3 (NVIDIA, AMD ou Intel Arc) e driver atualizado. UFC 1 a 4: processador Intel Core de 4ª geração (Haswell) ou AMD Ryzen, ou mais novo (o UFC 3 roda também em Intel Core de 2ª e 3ª geração pelo modo de compatibilidade do PORTHEON). UFC 5 e 6: processador com AVX2, 16 GB de RAM e, no UFC 6, 8 GB de memória de vídeo recomendados (perfis para placas de 4 e 6 GB).

## O que o PORTHEON faz

- Uma janela para todos os UFC: vídeo, jogabilidade e controles gravados direto em cada port.
- Configuração automática para o seu PC (processador, placa de vídeo, memória de vídeo, RAM e monitor), refeita quando o PC muda.
- Resolve sozinho os travamentos conhecidos: depois de a placa de vídeo parar (código 23) ou de uma falha no jogo (código 3), ele muda o que causou isso para o próximo JOGAR e explica em palavras.
- Controles: PS5, PS4, PS3, Xbox e genéricos; dois controles são dois jogadores nos UFC 1 a 4; Modo Parsec para jogar com um amigo.
- Perfis, backup automático dos saves, trapaças (UFC 4), atalhos na área de trabalho, 16 idiomas.

## Créditos

- **PORTHEON:** Marcelo Miyake (**XAndromedaXR**) — o launcher, a interface e as correções e otimizações de todos os ports.
- **Motor do UFC 5:** o build do KytyPS5 de **Cozmaa** e **Brotski**. **O XAndromedaXR melhorou o build deles** para o PORTHEON: a velocidade do jogo pelo FPS real, um build para AMD que resiste a falhas do driver, opções de torcida/sombras/arquibancada e o vigia de áudio desligado.
- **Motor do UFC 6:** o build do KytyPS5 do **Peepo**. **O XAndromedaXR melhorou o build dele** para o PORTHEON: luta na velocidade do PS5, vigia de áudio desligado, perfis de memória de vídeo para placas de 4, 6 e 8 GB e proteção contra travamentos de memória de vídeo.
- **Patch de FPS:** **lepexa_cell** (Lepexa). **O XAndromedaXR aprimorou o patch de FPS dele** para o PORTHEON. Os outros patches da comunidade: Lepexa e Antigravity.
- **Ports do UFC 1 a 4:** feitos a partir do port do UFC 3 de PS4 do **GNS** (GNSB), do SacredFire / bbport e do shadPS4.
- E também: KytyPS5 e Kyty (InoriRus), roshanarunk, Senaxx e EmK530 (trabalho no motor do UFC 5), deanzx06 (compatibilidade com Sandy/Ivy Bridge), GoldHEN Cheat Repository (trapaças do UFC 4), SDL_GameControllerDB.
- E também créditos para ken9l

## Licença

O PORTHEON e os ports são software livre sob a GNU GPL, versão 2 ou mais nova. O código-fonte é publicado com cada versão; a pasta `licenses` do pacote tem todas as licenças e os créditos.
