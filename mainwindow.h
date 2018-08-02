#ifndef AUDIOPLAYBACK_H
#define AUDIOPLAYBACK_H

#include <QAudioInput>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QAudioOutput>
#include <iostream>
#include <QByteArray>
#include <QBuffer>
#include <QCoreApplication>


class bufferPlayback: public QObject
{
    Q_OBJECT
public:
    ~bufferPlayback();
    bufferPlayback();
    qint64 bufferLength() const;

public slots:
    void startRecording();
    void startPlayback();
    void captureDataFromDevice();
    void captureDataIntoDevice();

signals:
    void mysignal();

private:
    qint64 audioLength(const QAudioFormat &format, qint64 microSeconds);
    QAudio::Mode        m_mode;

    void selectFormat();
    void stopPlayback();
    void initialize();
    void stopRecording();

    qint64              m_dataLengthRecord;
    qint64              m_dataLengthPlay;
    qint64              m_bufferLength;
    qint64              m_bytesReady;
    QAudioFormat        format;

    QAudioInput*        m_audioInput;
    QAudioDeviceInfo    m_audioInputDevice;
    QIODevice*          m_temp;
    QIODevice*          m_audioInputIODevice;

    QAudioDeviceInfo    m_audioOutputDevice;
    QAudioOutput*       m_audioOutput;
    qint64              m_playPosition;
    QBuffer             m_audioOutputIODevice;

    QByteArray          m_buffer;
    bool isOpen;

};

#endif // AUDIOPLAYBACK_H
