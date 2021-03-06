#include "inputwidget.h"

#ifdef QT_DEBUG
#include <QtDebug>
#endif

InputWidget::InputWidget(FFmpeg *ff, QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    ffmpeg = ff;
    _mediaInfo = new FFMediaInfo("",this);

    updateOptions();
}

FFMediaInfo *InputWidget::getMediaInfo()
{
    //update custom params before returning
    _mediaInfo->clearFFmpegOptions();

    for (int i = 0 ; i < _customParamEdits.count() ; i++)
    {
        QString param = _customParamEdits[i]->text();
        if (param != "")
        {
            QStringList option(param);
            option << _customValueEdits[i]->text();
            _mediaInfo->addFFmpegOption(option);
        }
    }

    return _mediaInfo;
}

void InputWidget::on_inputBrowseButton_clicked()
{
    QSettings settings;
    QString inputPath = QFileDialog::getOpenFileName(this,"Select the media file to transcode",settings.value("input/path",QVariant("")).toString());
    if (inputPath == "") return;

    _mediaInfo->updateInfo(ffmpeg->getMediaInfoString(inputPath));

    //Text
    QString mediaInfoString = "Media information";

    mediaInfoString += "\n\nContainers: " + _mediaInfo->extensions().join(",");

    if (_mediaInfo->duration() != 0.0)
    {
        QTime duration(0,0,0);
        duration = duration.addSecs(_mediaInfo->duration());
        mediaInfoString += "\nDuration: " + duration.toString("hh:mm:ss.zzz");
    }
    else if (_mediaInfo->isImageSequence())
    {
        mediaInfoString += "\nDuration: " + QString::number(_mediaInfo->frames().count()) + " frames";
        mediaInfoString += "\nStart Frame Number: " + QString::number(_mediaInfo->startNumber());
    }


    double size = _mediaInfo->size(FFMediaInfo::MB);
    int roundedSize = size*1000+0.5;
    size = roundedSize/1000;
    mediaInfoString += "\nSize: " + QString::number(size) + " MB";

    mediaInfoString += "\nContains video: ";
    if (_mediaInfo->hasVideo()) mediaInfoString += "yes";
    else mediaInfoString += "no";
    mediaInfoString += "\nContains audio: ";
    if (_mediaInfo->hasAudio()) mediaInfoString += "yes";
    else mediaInfoString += "no";
    mediaInfoString += "\nImage sequence: ";
    if (_mediaInfo->isImageSequence()) mediaInfoString += "yes";
    else mediaInfoString += "no";

    if (_mediaInfo->hasVideo())
    {
        mediaInfoString += "\n\nVideo codec: ";
        if(_mediaInfo->videoCodec() != nullptr)
        {
            mediaInfoString += _mediaInfo->videoCodec()->prettyName();
        }
        mediaInfoString += "\nResolution: " + QString::number(_mediaInfo->videoWidth()) + "x" + QString::number(_mediaInfo->videoHeight());
        mediaInfoString += "\nFramerate: " + QString::number(_mediaInfo->videoFramerate()) + " fps";
        int bitrate = _mediaInfo->videoBitrate(FFMediaInfo::Mbps);
        if (bitrate != 0) mediaInfoString += "\nBitrate: " + QString::number(bitrate) + " Mbps";
    }

    if (_mediaInfo->hasAudio())
    {
        mediaInfoString += "\n\nAudio codec: ";
        if(_mediaInfo->audioCodec() != nullptr)
        {
            mediaInfoString += _mediaInfo->audioCodec()->prettyName();
        }
        mediaInfoString += "\nSampling rate: " + QString::number(_mediaInfo->audioSamplingRate()) + " Hz";
        int abitrate = _mediaInfo->audioBitrate(FFMediaInfo::Kbps);
        if (abitrate != 0) mediaInfoString += "\nBitrate: " + QString::number(abitrate) + " Kbps";
    }

    //mediaInfoString += "\n\nFFmpeg analysis:\n" + _mediaInfo->ffmpegOutput();

    mediaInfosText->setText(mediaInfoString);

    //update UI
    inputEdit->setText(QDir::toNativeSeparators(_mediaInfo->fileName()));
    //keep in settings
    QFileInfo fi(inputPath);
    settings.setValue("input/path",QVariant(fi.path()));

    updateOptions();

    emit newMediaLoaded(_mediaInfo);
}

void InputWidget::on_addParamButton_clicked()
{
    //add a param and a value
    QLineEdit *customParam = new QLineEdit(this);
    customParam->setPlaceholderText("-param");
    customParam->setMinimumWidth(100);
    customParam->setMaximumWidth(100);
    //the value edit
    QLineEdit *customValue = new QLineEdit(this);
    customValue->setPlaceholderText("Value");
    //add to layout and lists
    customParamsLayout->insertRow(customParamsLayout->rowCount(),customParam,customValue);
    _customParamEdits << customParam;
    _customValueEdits << customValue;
}

void InputWidget::on_frameRateButton_toggled(bool checked)
{
    frameRateBox->setEnabled(checked);
    frameRateEdit->setEnabled(checked);
}

void InputWidget::on_frameRateBox_activated(const QString &arg1)
{
    if (arg1 != "Custom")
    {
        QString num = frameRateBox->currentText().replace(" fps","");
        frameRateEdit->setValue(num.toDouble());
    }
}

void InputWidget::on_frameRateEdit_valueChanged(double arg1)
{
    //update mediainfo
    _mediaInfo->setVideoFramerate(frameRateEdit->value());
    emit newMediaLoaded(_mediaInfo);
    //look for corresponding value
    for (int i = 1 ; i < frameRateBox->count() ; i++)
    {
        QString num = frameRateBox->itemText(i).replace(" fps","");
        if (num.toDouble() == arg1)
        {
            frameRateBox->setCurrentIndex(i);
            return;
        }
    }
    frameRateBox->setCurrentIndex(0);

}

void InputWidget::updateOptions()
{
    //frame rate
    frameRateButton->hide();
    frameRateBox->hide();
    frameRateEdit->hide();

    if (_mediaInfo->isImageSequence())
    {
        frameRateButton->show();
        frameRateBox->show();
        frameRateEdit->show();
    }

    //uncheck what is hidden
    if (frameRateButton->isHidden()) frameRateButton->setChecked(false);
}
