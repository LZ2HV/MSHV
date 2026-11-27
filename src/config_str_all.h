#ifndef _CONFIG_STR_ALL_H_
#define _CONFIG_STR_ALL_H_

#define COUNT_MODE 19

static const uint8_t pos_mods[COUNT_MODE] =
    {
        0,12,1,2,3,4,5,6,18,13,11,14,15,16,17,7,8,9,10
    };       
inline static const QString ModeStr(int i)
{
    QString ModeStr_[COUNT_MODE]={"MSK144","JTMS","FSK441","FSK315","ISCAT-A","ISCAT-B","JT6M",
                                 "JT65A","JT65B","JT65C","PI4","FT8","MSKMS","FT4",
                                 "Q65A","Q65B","Q65C","Q65D","FT2"};
    QString mode = "UNKNOWN";
    if (i>=0 && i<=COUNT_MODE-1)// stop exseptions 
        mode = ModeStr_[i];
    return  mode;
} 
inline static const QString ModeColorStr(int i)
{
    QString ModeColorStr_[COUNT_MODE]=
        {//"QLabel{background-color:rgb(145,220,255);}",   //Color MSK144
            "QLabel{background-color:rgb(145,220,255);",  //Color MSK144
            "QLabel{background-color :rgb(85,170,255);",   //Color JTMS
            "QLabel{background-color:rgb(255,255,0);",    //Color FSK441
            "QLabel{background-color:rgb(255,225,0);",    //Color FSK315
            "QLabel{background-color:rgb(204,255,255);",  //Color ISCAT-A
            "QLabel{background-color:rgb(204,255,255);",  //Color ISCAT-B
            "QLabel{background-color:rgb(255,0,255);",    //Color JT6M
            "QLabel{background-color:rgb(82,209,190);",   //Color JT65A//220, 160, 255  82, 209, 190
            "QLabel{background-color:rgb(82,209,190);",   //Color JT65B
            "QLabel{background-color:rgb(82,209,190);",   //Color JT65C
            "QLabel{background-color:rgb(0,230,0);",      //Color PI4
            "QLabel{background-color:rgb(92,235,220);",   //Color FT8
            "QLabel{background-color:rgb(145,220,255);",  //Color MSK144ms
            "QLabel{background-color:rgb(92,235,220);",   //Color FT4
            "QLabel{background-color:rgb(92,229,210);",   //Color Q65A
            "QLabel{background-color:rgb(92,229,210);",   //Color Q65B
            "QLabel{background-color:rgb(92,229,210);",   //Color Q65C
            "QLabel{background-color:rgb(92,229,210);",   //Color Q65D
            "QLabel{background-color:rgb(92,235,220);"};  //Color FT2
            //"QLabel{background-color:rgb(92,229,210);"};  //Color 19

    QString modec = "QLabel{background-color:rgb(255,255,255);}";// White
    if (i>=0 && i<=COUNT_MODE-1)// stop exseptions
        modec = ModeColorStr_[i];
    return  modec;
}    
#endif // __CONFIG_STR_ALL_H__
