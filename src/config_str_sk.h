#ifndef __CONFIG_STR_SK_H__
#define __CONFIG_STR_SK_H__

#define COUNT_SK 36
#define COUNT_LANGS 16

#if defined _SHKY_H_ 
static const QByteArray ShKey[COUNT_LANGS][COUNT_SK][2]=
	/////  EN  ////////////
    {{{"Ctrl+H","MSHV Help"},{"Ctrl+K","Keyboard Shortcuts"},{"Ctrl+O","File Open"},{"Alt+F4","Exit"},
    {"Ctrl+S","Sound Settings"},{"Ctrl+I","Interface Control"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Play Control"},{"Ctrl+R","Record Control"},{"Ctrl+L","View Log"},
    {"Alt+L","Add Qso To Log"},{"Alt+M","Start Monitor"},{"Alt+S","Stop Monitor"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","TX Stop"},{"Ctrl+A","Toggle Auto On/Off"},{"Ctrl+G","Generate Messages"},
    {"Ctrl+1","Save Display 1 Data as *.WAV"},{"Ctrl+2","Save Display 2 Data as *.WAV"},
    {"Alt+C","Clear Messages"},{"Ctrl+Z","Toggle ZAP On/Off"},
    {"F12","Screenshot"},{"F9","RX Only First Or Second Period"},
    {"F10","Switches number of displays from two to one and vice versa"},
    {"F11","Switches displays, if working on one display"},
    {"F8","View Spot Dialog"},{"Ctrl+`","Multi Answering Auto Seq Protocol DXpedition FT Q65"},// 26
    {"Ctrl+Q","Multi Answering Auto Seq Protocol Standard FT Q65"},// 27
    {"Ctrl+<","DF Tol Down (FT,Q65,JT65,PI4 Bandwidth)"},// 28
    {"Ctrl+>","DF Tol Up (FT,Q65,JT65,PI4 Bandwidth)"},// 29
    {"Ctrl+Click","On waterfall adjust RX and TX frequencies together FT,Q65"},// 30
    {"Shift+Click","On waterfall adjust TX frequency FT,Q65"},// 31
    {"Ctrl+W","Show/Hide Waterfall"},// 32
    {"Ctrl+T","Show/Hide TX Widget"},// 33
    {"Ctrl+ =","Band Up"},
    {"Ctrl+ -","Band Down"}},//34, 35      
    /////  BG  ////////////
    {{"Ctrl+H","MSHV Помощ"},{"Ctrl+K","Комбинации от клавиши"},{"Ctrl+O","Отваряне на файл"},{"Alt+F4","Изход"},
    {"Ctrl+S","Настройки на звука"},{"Ctrl+I","Интерфейс контрол"},{"Ctrl+M","Макроси"},
    {"Ctrl+P","Устройства за възпроизвеждане"},{"Ctrl+R","Устройства за запис"},{"Ctrl+L","Покажи Лога"},
    {"Alt+L","Добавяне на QSO в Лога"},{"Alt+M","Старт Монитор"},{"Alt+S","Стоп Монитор"},{"F1 до F7","Tx1 до Tx7"},
    {"Esc","TX Стоп"},{"Ctrl+A","Превключи Авто Вкл/Изкл"},{"Ctrl+G","Генериране на съобщения"},
    {"Ctrl+1","Запазване на Дисплей 1 във файл *.WAV"},{"Ctrl+2","Запазване на Дисплей 2 във файл *.WAV"},
    {"Alt+C","Изчистване на съобщения"},{"Ctrl+Z","Превключи ZAP Вкл/Изкл"},
    {"F12","Снимка на екрана"},{"F9","RX Само първи или втори период"},
    {"F10","Превключва броя на дисплеите от два на един и обратно"},
    {"F11","Превключва дисплеите, ако работи на един дисплей"},
    {"F8","Покажи Спот диалога"},{"Ctrl+`","Протокол Мулти-Отговор, DX Експедиция FT Q65"},// 26
    {"Ctrl+Q","Протокол Мулти-Отговор, Стандартен FT Q65"},// 27
    {"Ctrl+<","DF Tol надолу (FT,Q65,JT65,PI4 Лента на пропускане)"},// 28
    {"Ctrl+>","DF Tol нагоре (FT,Q65,JT65,PI4 Лента на пропускане)"},// 29
    {"Ctrl+Click","На водопада установява RX и TX честотите заедно FT,Q65"},// 30
    {"Shift+Click","На водопада установява честотата на TX FT,Q65"},// 31
    {"Ctrl+W","Покажи/Скрий Водопада"},// 32
    {"Ctrl+T","Покажи/Скрий TX Контролите"},// 33
    {"Ctrl+ =","Диапазон нагоре"},
    {"Ctrl+ -","Диапазон надолу"}},//34, 35            
    /////   RU   ////////////  
    {{"Ctrl+H","Помощь MSHV"},{"Ctrl+K","Горячие клавиши"},{"Ctrl+O","Открыть Файл WAW"},{"Alt+F4","Выход"},
    {"Ctrl+S","Настройка звука"},{"Ctrl+I","Интерфейс управления"},{"Ctrl+M","Макросы"},
    {"Ctrl+P","Воспроизводящее устройство"},{"Ctrl+R","Записывающее устройство"},{"Ctrl+L","Открыть Log MSHV"},
    {"Alt+L","Записать Qso в Log"},{"Alt+M","Старт Monitor"},{"Alt+S","Стоп Monitor"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","TX Stop"},{"Ctrl+A","Переключить Auto On/Off"},{"Ctrl+G","Создать сообщение"},
    {"Ctrl+1","Сохранить дисплей 1 в файл *.WAV"},{"Ctrl+2","Сохранить дисплей 2 в файл *.WAV"},
    {"Alt+C","Очистить сообщения"},{"Ctrl+Z","Переключить ZAP On/Off"},
    {"F12","Скриншот"},{"F9","RX Только 1-й Период или 2-й Период"},
    {"F10","Переключить количество дисплеев с 2-х на 1 и наоборот"},
    {"F11","Переключить дисплеи, если в работе один дисплей"},
    {"F8","Показать Спот диалог"},{"Ctrl+`","Мультиответ (Auto Seq)FT,Q65-DX экспедиция"},// 26
    {"Ctrl+Q","Мультиответ (Auto Seq)FT,Q65-Стандарт"},// 27
    {"Ctrl+<","DF Tol Меньше (FT,Q65,JT65,PI4 Полоса захвата)"},// 28
    {"Ctrl+>","DF Tol Больше (FT,Q65,JT65,PI4 Полоса захвата)"},// 29
    {"Ctrl+Click","(FT,Q65) На водопаде установить частоты RX и TX вместе"},// 30
    {"Shift+Click","(FT,Q65) На водопаде установить частоту TX"},// 31
    {"Ctrl+W","Скрыть/показать водопад"},// 32
    {"Ctrl+T","Скрыть/показать виджет TX"},// 33
    {"Ctrl+ =","Диапазон в верх"},
    {"Ctrl+ -","Диапазон в низ"}},//34, 35     
	/////   ZH   ////////////  
    {{"Ctrl+H","帮助 MSHV"},{"Ctrl+K","键盘快捷方式"},{"Ctrl+O","文件打开"},{"Alt+F4","退出"},
    {"Ctrl+S","声音设置"},{"Ctrl+I","接口控制"},{"Ctrl+M","自定义文字"},
    {"Ctrl+P","播放控制"},{"Ctrl+R","录音控制"},{"Ctrl+L","查看日志"},
    {"Alt+L","添加QSO到日志"},{"Alt+M","启动监听"},{"Alt+S","停止监听"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","停止 TX"},{"Ctrl+A","切换自动 开/关"},{"Ctrl+G","生成信息"},
    {"Ctrl+1","将显示屏1保存到文件 *.WAV"},{"Ctrl+2","将显示屏2保存到文件 *.WAV"},
    {"Alt+C","清除信息"},{"Ctrl+Z","切换 ZAP 开/关"},
    {"F12","截图"},{"F9","RX仅第一或第二周期"},
    {"F10","将显示器数量从两个切换到一个, 反之亦然"},
    {"F11","切换显示, 如果在一个显示器上工作"},
    {"F8","查看Spot对话"},{"Ctrl+`","多重应答自动程序协议 远征 FT Q65"},// 26
    {"Ctrl+Q","多重应答自动程序协议 标准 FT Q65"},// 27
    {"Ctrl+<","DF Tol 减少 (FT,Q65,JT65,PI4 带宽)"},// 28
    {"Ctrl+>","DF Tol 增加 (FT,Q65,JT65,PI4 带宽)"},// 29
    {"Ctrl+Click","在瀑布上一起调整 RX 和 TX 频率 FT,Q65"},// 30
    {"Shift+Click","在瀑布上调整 TX 频率 FT,Q65"},// 31
    {"Ctrl+W","显示/隐藏 瀑布"},// 32
    {"Ctrl+T","显示/隐藏 TX 小部件"},// 33
    {"Ctrl+ =","波段 向上"},
    {"Ctrl+ -","波段 向下"}},//34, 35          
    /////   ZHHK   ////////////  
    {{"Ctrl+H","説明 MSHV"},{"Ctrl+K","鍵盤快捷方式"},{"Ctrl+O","文件打開"},{"Alt+F4","退出"},
    {"Ctrl+S","聲音設置"},{"Ctrl+I","介面控制"},{"Ctrl+M","自定義文字"},
    {"Ctrl+P","播放控制"},{"Ctrl+R","錄音控制"},{"Ctrl+L","查看日誌"},
    {"Alt+L","添加到日志"},{"Alt+M","啟動監聽"},{"Alt+S","停止監聽"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","停止 TX"},{"Ctrl+A","切換自動 開/關"},{"Ctrl+G","產生信息"},
    {"Ctrl+1","將顯示器1數據保存為 *.WAV"},{"Ctrl+2","將顯示器2數據保存為 *.WAV"},
    {"Alt+C","清除信息"},{"Ctrl+Z","切換 ZAP 開/關"},
    {"F12","截圖"},{"F9","RX僅第一或第二週期"},
    {"F10","將顯示器數量從兩個切換到一個, 反之亦然"},
    {"F11","切換顯示, 如果在一個顯示器上工作"},
    {"F8","查看Spot對話"},{"Ctrl+`","多重應答自動程序協定 遠征 FT Q65"},// 26
    {"Ctrl+Q","多重應答自動程序協定 標準 FT Q65"},// 27
    {"Ctrl+<","DF Tol 減少 (FT,Q65,JT65,PI4 頻寬)"},// 28
    {"Ctrl+>","DF Tol 增加 (FT,Q65,JT65,PI4 頻寬)"},// 29
    {"Ctrl+Click","在瀑布上一起調整 RX 和 TX 頻率 FT,Q65"},// 30
    {"Shift+Click","在瀑布上調整 TX 頻率 FT,Q65"},// 31
    {"Ctrl+W","顯示/隱藏 瀑布"},// 32
    {"Ctrl+T","顯示/隱藏 TX 小部件"},// 33
    {"Ctrl+ =","波段 向上"},
    {"Ctrl+ -","波段 向下"}},    	
    /////   ESES   ////////////  ok	
    {{"Ctrl+H","Ayuda MSHV"},{"Ctrl+K","Atajos de Teclado"},{"Ctrl+O","Abrir Fichero"},{"Alt+F4","Salir"},
    {"Ctrl+S","Ajustes de Sonido"},{"Ctrl+I","Control de Interface"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Control de Repreducción"},{"Ctrl+R","Control de Grabación"},{"Ctrl+L","Ver Log"},
    {"Alt+L","Añadir QSO al Log"},{"Alt+M","Iniciar Monitor"},{"Alt+S","Parar Monitor"},{"F1 a F7","Tx1 a Tx7"},
    {"Esc","Parar TX"},{"Ctrl+A","Alternar encendido / apagado automático"},{"Ctrl+G","Generar Mensajes"},
    {"Ctrl+1","Guardar datos de la pantalla 1 como *.wav"},{"Ctrl+2","Guardar datos de la pantalla 2 como *.wav"},
    {"Alt+C","Borrar Mensajes"},{"Ctrl+Z","Toogle ZAP On/Off"},
    {"F12","Captura de Pantalla"},{"F9","RX sólo primer o segundo periodo"},
    {"F10","Cambia el número de pantallas de dos a uno y viceversa"},
    {"F11","Cambia las pantallas, si trabaja en una pantalla"},
    {"F8","Ver cuadro de diálogo Spot"},{"Ctrl+`","Protocolo de secuencia automática de respuesta múltiple Dxpedition FT Q65"},// 26
    {"Ctrl+Q","Protocolo de secuencia automática de respuesta múltiple estándar FT Q65"},// 27
    {"Ctrl+<","DF Tol Down (FT,Q65,JT65,PI4 Bandwidth)"},// 28
    {"Ctrl+>","DF Tol Up (FT,Q65,JT65,PI4 Bandwidth)"},// 29
    {"Ctrl+Click","En waterfall, ajuste las frecuencias RX y TX juntas FT, Q65"},// 30
    {"Shift+Click","En waterfall ajustar la frecuencia de TX FT, Q65"},// 31
    {"Ctrl+W","Mostrar / Ocultar waterfall"},// 32
    {"Ctrl+T","Mostrar / Ocultar widget de TX"},// 33
    {"Ctrl+ =","Subir Banda"},
    {"Ctrl+ -","Bajar Banda"}},//34, 35	    
    /////   CAES   ////////////  ok	
    {{"Ctrl+H","Ajuda MSHV"},{"Ctrl+K","Dreceres de Teclat"},{"Ctrl+O","Obrir Fitxer"},{"Alt+F4","Sortir"},
    {"Ctrl+S","Configuració de So"},{"Ctrl+I","Control d'Interface"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Control de Reproducció"},{"Ctrl+R","Control de Gravació"},{"Ctrl+L","Veure Log"},
    {"Alt+L","Afegir QSO al Log"},{"Alt+M","Iniciar Monitor"},{"Alt+S","Parar Monitor"},{"F1 a F7","Tx1 a Tx7"},
    {"Esc","Parar TX"},{"Ctrl+A","Activa o desactiva l’encés automàtic"},{"Ctrl+G","Generar Missatges"},
    {"Ctrl+1","Guardar dades de la pantalla 1 com a *.wav"},{"Ctrl+2","Guardar dades de la pantalla 2 com a *.wav"},
    {"Alt+C","Borrar Missatges"},{"Ctrl+Z","Toogle ZAP On/Off"},
    {"F12","Captura de Pantalla"},{"F9","RX només primer o segon període"},
    {"F10","Canvia el número de pantalles de dos a una i viceversa"},
    {"F11","Canvia les pantalles, si treballa en una pantalla"},
    {"F8","Veure quadre de diàles Spot"},{"Ctrl+`","Protocol de seqüència automàtica de resposta múltiple Dxpedition FT Q65"},// 26
    {"Ctrl+Q","Protocol de seqüència automàtica de resposta múltiple estàndar FT Q65"},// 27
    {"Ctrl+<","DF Tol Down (FT,Q65,JT65,PI4 Bandwidth)"},// 28
    {"Ctrl+>","DF Tol Up (FT,Q65,JT65,PI4 Bandwidth)"},// 29
    {"Ctrl+Click","En waterfall, ajusti les freqüències RX y TX juntas FT, Q65"},// 30
    {"Shift+Click","En waterfall ajustar la frecuencia de TX FT, Q65"},// 31
    {"Ctrl+W","Mostrar / Ocultar waterfall"},// 32
    {"Ctrl+T","Mostrar / Ocultar widget de TX"},// 33
    {"Ctrl+ =","Pujar Banda"},
    {"Ctrl+ -","Baixar Banda"}},    
    /////   PTPT   ////////////  ok	    	
	/*{{"Ctrl+H","Ajuda MSHV"},{"Ctrl+K","Teclas de Atalho"},{"Ctrl+O","Abrir Ficheiro"},{"Alt+F4","Sair"},
    {"Ctrl+S","Configurações de Áudio"},{"Ctrl+I","Controlo da Interface"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Saída de Áudio"},{"Ctrl+R","Entrada de Áudio"},{"Ctrl+L","Ver Log"},
    {"Alt+L","Adicionar QSO ao Log"},{"Alt+M","Iniciar Monitor"},{"Alt+S","Parar Monitor"},{"F1 a F7","Tx1 a Tx7"},
    {"Esc","Parar TX "},{"Ctrl+A","Alternar Auto On/Off"},{"Ctrl+G","Aplicar Mensagens em TX"},
    {"Ctrl+1","Gravar Áudio do Monitor 1"},{"Ctrl+2","Gravar Áudio do Monitor 2"},
    {"Alt+C","Limpar Mensagens"},{"Ctrl+Z","Alternar ZAP On/Off"},
    {"F12","Captura do Ecrã"},{"F9","RX Apenas no Primeiro ou no Segundo Período"},
    {"F10","Muda o Número de Ecrãs de Dois para Um ou Vice-Versa"},
    {"F11","Muda de Ecrãs, se Estiver a Trabalhar num Só"},
    {"F8","Caixa de Diálogo para Envio de Spot"},{"Ctrl+`","Protocolo Sequencial Automático de Resposta a Expedições DX FT & Q65"},
    {"Ctrl+Q","Protocolo Sequencial Automático de Resposta Predefinida FT & Q65"},
    {"Ctrl+<","DF Tol Para Baixo (FT, Q65, JT65, PI4 Largura de Banda)"},
    {"Ctrl+>","DF Tol Para Cima (FT, Q65, JT65, PI4 Largura de Banda)"},
    {"Ctrl+Click","Ajustar Ambas as Frequências TX e RX (FT & Q65) na Waterfall"},
    {"Shift+Click","Ajustar a Frequência TX (FT & Q65) na Waterfall"},
    {"Ctrl+W","Exibir/Ocultar: Waterfall"},
    {"Ctrl+T","Exibir/Ocultar: Painel TX"},
    {"Ctrl+ =","Subir na Banda"},
    {"Ctrl+ -","Descer na Banda"}},2026-03-06*/
    {{"Ctrl+H","Ajuda MSHV"},{"Ctrl+K","Teclas de atalho"},{"Ctrl+O","Abrir ficheiro"},{"Alt+F4","Sair"},
    {"Ctrl+S","Definições de áudio"},{"Ctrl+I","Configuração da Interface"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Saída de áudio"},{"Ctrl+R","Entrada de áudio"},{"Ctrl+L","Consultar Log"},
    {"Alt+L","Adicionar QSO ao Log"},{"Alt+M","Iniciar monitor"},{"Alt+S","Parar monitor"},{"F1 a F7","Tx1 a Tx7"},
    {"Esc","Parar TX "},{"Ctrl+A","Alternar auto Ligar/Desligar"},{"Ctrl+G","Aplicar mensagens em TX"},
    {"Ctrl+1","Guardar áudio formato (WAV)"},{"Ctrl+2","Guardar áudio formato (WAV)"},
    {"Alt+C","Limpar mensagens"},{"Ctrl+Z","Alternar ZAP Ligar/Desligar"},
    {"F12","Captura do ecrã"},{"F9","RX apenas no 1.º ou 2.º período"},
    {"F10","Alternar entre um ou dois ecrãs"},
    {"F11","Mudar de ecrã (se usar apenas um)"},
    {"F8","Janela de envio de Spot"},{"Ctrl+`","Protocolo automático de expedição DX (FT & Q65)"},
    {"Ctrl+Q","Protocolo de resposta automático (FT & Q65)"},
    {"Ctrl+<","Diminuir tolerância DF (FT/Q65/JT/PI4)"},
    {"Ctrl+>","Aumentar tolerância DF (FT/Q65/JT/PI4)"},
    {"Ctrl+Click","Ajustar ambas as frequências TX e RX (FT & Q65) na Waterfall"},
    {"Shift+Click","Ajustar a frequência TX (FT & Q65) na Waterfall"},
    {"Ctrl+W","Mostrar/Ocultar: Waterfall"},
    {"Ctrl+T","Mostrar/Ocultar: Painel TX"},
    {"Ctrl+ =","Subir banda"},
    {"Ctrl+ -","Descer banda"}},    	    		 	
    /////   RORO   ////////////  ok	
	{{"Ctrl+H","MSHV Ajutor"},{"Ctrl+K","Keyboard Shortcuts"},{"Ctrl+O","DEschide fisierul"},{"Alt+F4","Iesire"},
	{"Ctrl+S","Setare sunet"},{"Ctrl+I","Control Interfata"},{"Ctrl+M","Mesaje"},
    {"Ctrl+P","Control iesire audio"},{"Ctrl+R","Control intrare audio"},{"Ctrl+L","Vezi Log"},
    {"Alt+L","Adauga QSO la Log"},{"Alt+M","Start Monitor"},{"Alt+S","Stop Monitor"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","TX Stop"},{"Ctrl+A","Schimba Auto On/Off"},{"Ctrl+G","Genereaza Mesaje"},
    {"Ctrl+1","Salveaza Display 1 Data ca *.WAV"},{"Ctrl+2","Salveaza Display 2 Data ca *.WAV"},
    {"Alt+C","Sterge Mesajele"},{"Ctrl+Z","Schimba ZAP On/Off"},
    {"F12","Screenshot"},{"F9","RX numai First Or Second Period"},
    {"F10","Schimba numarul de ferestre le la doi la unu si vice versa"},
    {"F11","Schimba fereastra, daca se lucreaza cu o fereastra"},
    {"F8","Vezi Spot Dialog"},{"Ctrl+`","Multi raspuns autosecventa Protocol pentru DXpeditie FT, Q65"},
    {"Ctrl+Q","Multi raspuns autosecventa Protocol pentru QSO Standard FT, Q65"},
    {"Ctrl+<","DF Tol scade (FT,Q65,JT65,PI4 Banda de trecere)"},
    {"Ctrl+>","DF Tol creste (FT,Q65,JT65,PI4 Banda de trecere)"},
    {"Ctrl+Click","Pe waterfall corecteaza ambele frecvente la RX si TX pentru FT,Q65"},
    {"Shift+Click","On waterfall adjust TX frequency FT,Q65"},
    {"Ctrl+W","Arata/ascunde Waterfall"},
    {"Ctrl+T","arata/ascunde casetele de jos"},
    {"Ctrl+ =","Banda Up"},
    {"Ctrl+ -","Banda Down"}},    	
    /////   DADK   ////////////  ok
	{{"Ctrl+H","MSHV Hjælp"},{"Ctrl+K","Tastetur Genveje"},{"Ctrl+O","Fil Åben"},{"Alt+F4","Exit"},
	{"Ctrl+S","Lyd Indstillinger"},{"Ctrl+I","Interface kontrol"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Afspille Kontrol"},{"Ctrl+R","Optage Kontrol"},{"Ctrl+L","Se Log"},
    {"Alt+L","Send Qso Til Log"},{"Alt+M","Start Monitor"},{"Alt+S","Stop Monitor"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","TX Stop"},{"Ctrl+A","Skift Auto On/Off"},{"Ctrl+G","Generer Meddelser"},
    {"Ctrl+1","Gem Display 1 Data som *.WAV"},{"Ctrl+2","Dem Display 2 Data som *.WAV"},
    {"Alt+C","Slet Meddelser"},{"Ctrl+Z","Skift ZAP On/Off"},
    {"F12","Screenshot"},{"F9","RX Kun Første Eller Anden Periode"},
    {"F10","Skifter antal displays fra to til et og så fremdeles"},
    {"F11","Skifter displays, hvis man arbejder med et display"},
    {"F8","Vis Spot Dialog"},{"Ctrl+`","Multi Kanal Auto Seq Protokol DXpedition FT Q65"},
    {"Ctrl+Q","Multi Kanal Auto Seq Protokol Standard FT Q65"},
    {"Ctrl+<","DF Tol Ned (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+>","DF Tol Op (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+Click","På vandfald samles RX and TX frekvenser til samme FT,Q65"},
    {"Shift+Click","På vandfald justeres TX frekvens FT,Q65"},
    {"Ctrl+W","Vis/Gem Vandfald"},
    {"Ctrl+T","Vis/Gem TX Widget"},
    {"Ctrl+ =","Bånd Up"},
    {"Ctrl+ -","Bånd Down"}},
	/////  PLPL  //////////// ok
    {{"Ctrl+H","Pomoc MSHV"},{"Ctrl+K","Klawisze Skrótów"},{"Ctrl+O","Otwórz Plik"},{"Alt+F4","Wyjście"},
    {"Ctrl+S","Ustawienie Dźwięku"},{"Ctrl+I","Ustawienie interfejsu"},{"Ctrl+M","Makra"},
    {"Ctrl+P","Ustawienie odtwarzania"},{"Ctrl+R","Ustawienie nagrywania"},{"Ctrl+L","Pokaż Dziennik"},
    {"Alt+L","Dodaj QSO do Dziennika"},{"Alt+M","Start Monitor"},{"Alt+S","Stop Monitor"},{"F1 to F7",	"Tx1 to Tx7"},
    {"Esc",	"TX Stop"},{"Ctrl+A","Przełącz Auto Wł./Wył"},{"Ctrl+G","Generuj Pakiety"},
    {"Ctrl+1","Zapisz Obraz 1 jako Dane *.WAV"},{"Ctrl+2","Zapisz Obraz 2 jako Dane *.WAV"},
    {"Alt+C","Wyczyść Pakiety"},{"Ctrl+Z","Przełącz ZAP Wł./Wył"},
    {"F12","Zrzut Ekranu"},{"F9","RX Tylko Pierwszy Lub Drugi Cykl"},
    {"F10","Przełącza liczbę monitorów z dwóch na jeden i odwrotnie"},
    {"F11","Przełącza monitory, jeśli pracujesz na jednym ekranie"},
    {"F8","Pokaż Okno Spotów"},{"Ctrl+`","Multi Answering Auto Seq Protocol DXpedition FT Q65"},
    {"Ctrl+Q","Multi Answering Auto Seq Protocol Standard FT Q65"},
    {"Ctrl+<","DF Tol Mniej (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+>","DF Tol Więcej (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+Click","Na wodospadzie dostosuj jednocześnie częstotliwości RX i TX FT,Q65"},
    {"Shift+Click","Na wodospadzie dzostosuj częstotliwość TX FT,Q65"},
    {"Ctrl+W","Pokaż/Ukryj Wodospad"},
    {"Ctrl+T","Pokaż/Ukryj TX Widżet"},
    {"Ctrl+ =","Pasmo Wyżej"},
    {"Ctrl+ -","Pasmo Niżej"}},    	
	/////  FRFR  ////////////
	{{"Ctrl+H","Aide"},{"Ctrl+K","Raccourcis clavier"},{"Ctrl+O","Ouvrir un fichier"},{"Alt+F4","Sortir"},
    {"Ctrl+S","Paramètres de sons"},{"Ctrl+I","Interface de contrôle"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Contrôle de lecture"},{"Ctrl+R","Contrôle d'enregistrement"},{"Ctrl+L","Affichage du Log"},
    {"Alt+L","Ajoute le qso au Log"},{"Alt+M","Démarrer le monitoring"},{"Alt+S","Stopper le monitoring"},{"F1 to F7","Tx1 à Tx7"},
    {"Esc","TX Stop"},{"Ctrl+A","Auto On/Off"},{"Ctrl+G","Générer les messages"},
    {"Ctrl+1","Sauvegarder l'affichage 1 en data sous *.WAV"},{"Ctrl+2","Sauvegarder l'affichage en data sous *.WAV"},
    {"Alt+C","Effacer les messages"},{"Ctrl+Z","ZAP On/Off"},
    {"F12","Capture d'écran"},{"F9","RX seulement en 1ère ou 2ème période"},
    {"F10","Switcher les écrans de 1 à 2 ou vice versa"},
    {"F11","Switcher les écrans, si fonctionne sur 1 seul écran"},
    {"F8","Affiche les infos des spots"},{"Ctrl+`","Séquence automatique du protocole de multi-réponse DXpedition en mode FT Q65"},
    {"Ctrl+Q","Séquence automatique du protocole de multi-réponse Standard en mode FT Q65"},
    {"Ctrl+<","Diminuer Tolérance DF (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+>","Augmenter la tolérance DF (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+Click","Ajustement des fréquences RX et TX sur la chute d'eau en modes FT,Q65"},
    {"Shift+Click","Ajustement de la fréquence TX sur la chute d'eau en modes FT,Q65"},
    {"Ctrl+W","Afficher/Cacher la chute d'eau"},
    {"Ctrl+T","Afficher/Cacher TX Widget"},
    {"Ctrl+ =","Bande supérieure"},
    {"Ctrl+ -","Bande inférieure"}},
    /////   PTBR   ////////////  		
	{{"Ctrl+H","Ajuda do MSHV"},{"Ctrl+K","Teclas de Atalho"},{"Ctrl+O","Abrir Arquivo"},{"Alt+F4","Sair"},
    {"Ctrl+S","Configurações de Áudio"},{"Ctrl+I","Controle da Interface"},{"Ctrl+M","Macros"},
    {"Ctrl+P","Saída de Áudio"},{"Ctrl+R","Entrada de Áudio"},{"Ctrl+L","Visualizar Log"},
    {"Alt+L","Adicionar QSO ao Log"},{"Alt+M","Iniciar Monitor"},{"Alt+S","Parar Monitor"},{"F1 a F7","Tx1 a Tx7"},
    {"Esc","Parar TX"},{"Ctrl+A","Alternar Auto (On/Off)"},{"Ctrl+G","Aplicar Mensagens ao TX"},
    {"Ctrl+1","Gravar Áudio do Monitor 1"},{"Ctrl+2","Gravar Áudio do Monitor 2"},
    {"Alt+C","Limpar Mensagens"},{"Ctrl+Z","Alternar ZAP (On/Off)"},
    {"F12","Capturar Tela"},{"F9","RX somente no primeiro ou segundo períodos"},
    {"F10","Alternar Telas"},
    {"F11","Muda Telas (se somente uma em uso)"},
    {"F8","Enviar Spot"},{"Ctrl+`","Selecionar Resposta Predefinida (Expedições DX FT & Q65)"},
    {"Ctrl+Q","Selecionar Resposta Predefinida (FT & Q65)"},
    {"Ctrl+<","DF Tol Para Baixo (FT, Q65, JT65, PI4 Largura de Banda)"},
    {"Ctrl+>","DF Tol Para Cima (FT, Q65, JT65, PI4 Largura de Banda)"},
    {"Ctrl+Click","Ajustar Ambas as Frequências TX e RX (FT & Q65) na Waterfall"},
    {"Shift+Click","Ajustar a Frequência TX (FT & Q65) na Waterfall"},
    {"Ctrl+W","Exibir/ocultar: Waterfall"},
    {"Ctrl+T","Exibir/ocultar: Painel TX"},
    {"Ctrl+ =","Subir na faixa"},
    {"Ctrl+ -","Descer na faixa"}},
	/////   NBNO   ////////////
	{{"Ctrl+H","MSHV Hjelp"},{"Ctrl+K","Tastatursnarveier"},{"Ctrl+O","Åpne fil"},{"Alt+F4","Avslutt"},
    {"Ctrl+S","Lydinnstillinger"},{"Ctrl+I","Grensesnittkontroll"},{"Ctrl+M","Makroer"},
    {"Ctrl+P","Avspillings Kontroll"},{"Ctrl+R","Opptaks Kontroll"},{"Ctrl+L","Se Logg"},
    {"Alt+L","Legg til Qso til logg"},{"Alt+M","Start Monitor"},{"Alt+S","Stop Monitor"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","TX Stopp"},{"Ctrl+A","Slå Auto Av/På"},{"Ctrl+G","Generer meldinger"},
    {"Ctrl+1","Lagre Display 1 Data som *.WAV"},{"Ctrl+2","Lagre Display 2 Data som *.WAV"},
    {"Alt+C","Fjern meldinger"},{"Ctrl+Z","Slå ZAP av/på"},
    {"F12","Skjermdump"},{"F9","RX Bare første eller andre periode"},
    {"F10","Bytter skjerm visning fra to til en og omvendt"},
    {"F11","Bytter skjerm visning, hvis du arbeider på én skjerm"},
    {"F8","Se Spot -dialogboksen"},{"Ctrl+`","Multi Svar Auto Seq Protokoll DXpedition FT Q65"},
    {"Ctrl+Q","Multi Svar Auto Seq Protokoll Standard FT Q65"},
    {"Ctrl+<","DF Tol Ned (FT,Q65,JT65,PI4 Båndbredde)"},
    {"Ctrl+>","DF Tol Opp (FT,Q65,JT65,PI4 Båndbredde)"},
    {"Ctrl+Click","Juster spektrogram RX og TX frekvenser FT, Q65"},
    {"Shift+Click","Juster spektrogram TX frekvens FT,Q65"},
    {"Ctrl+W","Vis/Skjul spektrogram"},
    {"Ctrl+T","Vis/Skjul TX Widget"},
    {"Ctrl+ =","Bånd Opp"},
    {"Ctrl+ -","Bånd Ned"}},
	/////  ITIT  ////////////
    {{"Ctrl+H","MSHV Help"},{"Ctrl+K","Tasti Rapidi"},{"Ctrl+O","File Apri"},{"Alt+F4","Esci"},
    {"Ctrl+S","Impostazioni Audio"},{"Ctrl+I","Controllo Interfacccia"},{"Ctrl+M","Macro"},
    {"Ctrl+P","Play Controllo"},{"Ctrl+R","Record Controllo"},{"Ctrl+L","Vedi Log"},
    {"Alt+L","Aggiungi Qso Al Log"},{"Alt+M","Start Monitor"},{"Alt+S","Stop Monitor"},{"F1 to F7","Tx1 to Tx7"},
    {"Esc","TX Stop"},{"Ctrl+A","Attiva/Disattiva Auto On/Off"},{"Ctrl+G","Genera Messaggi"},
    {"Ctrl+1","Salva Display 1 Data come *.WAV"},{"Ctrl+2","Salva Display 2 Data come *.WAV"},
    {"Alt+C","Pulisci Messaggi"},{"Ctrl+Z","Attiva/Disattiva ZAP On/Off"},
    {"F12","Cattura Schermo"},{"F9","RX Solo Primo O Secondo Periodo"},
    {"F10","Commuta il numero di display da due a uno e viceversa"},
    {"F11","Cambia display, se si lavora su un display"},
    {"F8","Vedi Spot Dialog"},{"Ctrl+`","Risposta Multipla ad Auto Seq Protocollo DXpedition FT Q65"},
    {"Ctrl+Q","Risposta Multipla ad Auto Seq Protocollo Standard FT Q65"},
    {"Ctrl+<","DF Tol Giù (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+>","DF Tol Su (FT,Q65,JT65,PI4 Bandwidth)"},
    {"Ctrl+Click","Sul waterfall regola le frequenze RX e TX insieme FT,Q65"},
    {"Shift+Click","Sul waterfall regola la frequenza TX FT,Q65"},
    {"Ctrl+W","Vedi/Nascondi il Waterfall"},
    {"Ctrl+T","Vedi/Nascondi TX Widget"},
    {"Ctrl+ =","Banda Su"},
    {"Ctrl+ -","Banda Giù"}},
	/////  CSCZ  ////////////
    {{"Ctrl+H","MSHV Nápověda"},{"Ctrl+K","Klávesové zkratky"},{"Ctrl+O","Otevření souboru"},{"Alt+F4","Konec"},
    {"Ctrl+S","Nastavení zvuku"},{"Ctrl+I","Nastavení intreface"},{"Ctrl+M","Makra"},
    {"Ctrl+P","Nastavení přehrávání"},{"Ctrl+R","nastavení nahrávání"},{"Ctrl+L","Zobraz LOG"},
    {"Alt+L","Přidat QSO do LOGu"},{"Alt+M","Start Monitor"},{"Alt+S","Stop Monitor"},{"F1 to F7","Tx1 do Tx7"},
    {"Esc","TX Stop"},{"Ctrl+A","Přepínání automatického zapnutí/vypnutí"},{"Ctrl+G","Generování zprávy"},
    {"Ctrl+1","Ulož data z obrazovky 1 jako *.WAV"},{"Ctrl+2","Ulož data z obrazovky 2 jako *.WAV"},
    {"Alt+C","Vymazání zprávy"},{"Ctrl+Z","Přepínání ZAP zapnuto/vypnuto"},
    {"F12","Uložení obrazovky"},{"F9","RX Pouze první nebo druhá perioda"},
    {"F10","Přepnutí počtu displejů ze dvou na jeden a naopak"},
    {"F11","Přepíná displeje, pokud pracuje na jednom displeji"},
    {"F8","Zobrazit dialogové okno Spot"},{"Ctrl+`","Multi Answering Auto Seq Protocol DXpedition FT Q65"},
    {"Ctrl+Q","Multi Answering Auto Seq Protocol Standard FT Q65"},
    {"Ctrl+<","DF Tol dolů (FT,Q65,JT65,PI4 šířka pásma)"},
    {"Ctrl+>","DF Tol nahoru (FT,Q65,JT65,PI4 šířka pásma)"},
    {"Ctrl+Click","Při vodopádu nastaví frekvence RX a TX společně FT,Q65"},
    {"Shift+Click","Na vodopádu nastaví frekvenci TX FT,Q65"},
    {"Ctrl+W","Zobraz/Skryj vodopád"},
    {"Ctrl+T","Zobraz/Skryj TX Widget"},
    {"Ctrl+ =","Pásmo nahoru"},
    {"Ctrl+ -","Pásmo dolů"}}	 		    	    	
    };   	    
#endif 
  
#if defined _MOUNTH_H_ 
static const QByteArray IntToMonth[COUNT_LANGS][13] =
    {
    	/////   EN   ////////////
        {"None","Jan","Feb","Mar","Apr","May","June","July","Aug","Sept","Oct","Nov","Dec"},
        /////   BG   ////////////
        {"None","Ян.","Февр.","Март","Апр.","Май","Июни","Июли","Авг.","Септ.","Окт.","Ноем.","Дек."},
        /////   RU   ////////////
        {"None","Января","Февраля","Марта","Апреля","Мая","Июня","Июля","Августа","Сентября","Октября","Ноября","Декабря"},   
        /////   ZH   ////////////   
        {"None","一月","二月","三月","四月","五月","六月","七月","八月","九月","十月","十一月","十二月"}, 
        /////   ZHHK   //////////   
        {"None","一月","二月","三月","四月","五月","六月","七月","八月","九月","十月","十一月","十二月"}, 
        /////   ESES   ////////////
        {"None","En","Feb","Mar","Abr","May","Jun","Jul","Ag","Sept","Oct","Nov","Dic"},
        /////   CAES   ////////////
        {"None","Gen","Febr","Març","Abr","Maig","Juny","Jul","Ag","Set","Oct","Nov","Des"},
        /////   PTPT   ////////////
        {"None","Jan","Fev","Mar","Abr","Mai","Jun","Jul","Ago","Set","Out","Nov","Dez"},
        /////   RORO   ////////////
        {"None","Ian","Feb","Mar","Apr","Mai","Iun","Iul","Aug","Sept","Oct","Nov","Dec"},
        /////   DADK   ////////////
        {"None","Jan","Feb","Mar","Apr","Maj","Juni","Juli","Aug","Sept","Okt","Nov","Dec"},       
    	/////   PLPL   ////////////
        {"None","Sty","Lut","Mar","Kwi","Maj","Czer","Lip","Sie","Wrze","Paź","Lis","Gru"},
        /////   FRFR   ////////////
        {"None","Jan","Fev","Mar","Avr","Mai","Juin","Juil","Aout","Sept","Oct","Nov","Dec"},
        /////   PTBR   ////////////
        {"None","Jan","Fev","Mar","Abr","Mai","Jun","Jul","Ago","Set","Out","Nov","Dez"},
        /////   NBNO   ////////////	
        {"None","Jan","Feb","Mar","Apr","Mai","Juni","Juli","Aug","Sept","Okt","Nov","Des"},	        	 
        /////   ITIT  ////////////
        {"None","Gen","Feb","Mar","Apr","Mag","Giu","Lug","Ago","Set","Ott","Nov","Dic"},
        /////  CSCZ  ////////////
        {"Žádné","Led","Úno","Bře","Dub","Kvě","Črv","Čvc","Srp","Zář","Říj","Lis","Pro"}	                 
    };     
#endif         
#endif // __CONFIG_STR_SK_H__
