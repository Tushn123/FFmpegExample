#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    /**
     * 读取本地视频文件，不进行编解码，提取aac音频，并写入adts头
     *
     * @brief videoExtractAAC
     * @param videoFile  输入的视频文件
     * @param aacFile    输出的aac文件
     * @return
     */
    bool videoExtractAAC(const char* videoFile,const char* aacFile);
    int adts_header(char* adts_header,
                    int data_length,
                    int profile,
                    int samplerate,
                    int channels);
    void testVideoExtractAAC();




private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
