# capturer

в репозитории несколько проектов:
- capturer - основной проект. программа для захвата видео с просмотром входного сигнала с низкой задержкой, что полезно при записи игрового видео для чего, собственно, программа и писалась
- android_ctrl - дистанционное управление программой с ведройда
- remote_audio_output - программа для вывода звука по сети

### важно!11
необходима плата захвата с поддержкой интерфейса decklink. хотя поддерживаются платы с интерфесами video4linux/dshow, но это направление не основное и с какими-то платами работает норм, а с какими-то глючит

### сборка программ
все программы написана с использованием библиотек Qt5. соответственно требуется среда Qt Creator, а при открытии проекта рекомендуется отключить опцию теневой сборки. программы собираются как и любые qt-проекты, некоторые особенности сборки есть толко у программы capturer. итак:

- лучше собирать только 64-разрядную версию программы. 32-х битная почти наверняка будет вылетать с исключением std::bad_alloc при попытке начать запись - все зависит от комбинации разрешения, пиксельного формата и выбранного энкодера

- опционально. если нужна возможность управлять программой с ведройда через capturer_ctrl, то выполнить скрипт ./externals/3rdparty/http_server_update.(sh/cmd) для загрузки репозиториев с реализацией http-сервера

##### под линух
- с сайта https://www.blackmagicdesign.com/support загрузить "Desktop Video SDK"
- распаковать sdk в каталог ./externals/3rdparty/blackmagic_decklink_sdk
- собрать ffmpeg - для этого из каталога ./externals/3rdparty/ffmpeg запустить скрипт build.sh
<br>для работы скрипта необходимы пакеты: <b>autoconf automake libtool build-essential cmake git subversion mercurial</b>
<br>скрипт проверялся в дистрибутивах Ubuntu 16.04, 17.10 и Fedora 25-26, но периодически что-нибудь отваливается, тогда приходится разбираться по ситуации

##### под венду
- с сайта https://ffmpeg.zeranoe.com/builds загрузить shared и dev пакеты ffmpeg
- dev раcпаковать в ./externals/3rdparty/ffmpeg
- dll-ки из shared распаковать в ./bin
<br><br>
- собрать