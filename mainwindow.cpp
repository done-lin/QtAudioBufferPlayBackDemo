#include "mainwindow.h"

// The buffer length is assigned in audioLength function
// now it is set the audio duraton length and get the audio length thereafter

const qint64 BufferDurationUs       = 10 * 1000000;  // 10second

bufferPlayback::bufferPlayback()
    :   m_mode(QAudio::AudioInput)
    ,   m_audioInput(0)
    ,   m_audioInputDevice(QAudioDeviceInfo::defaultInputDevice())
    ,   m_audioInputIODevice(0)
    ,   m_audioOutputDevice(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_audioOutput(0)
    ,   m_bufferLength(0)
    ,   m_dataLengthPlay(0)
    ,   m_dataLengthRecord(0)
    ,   isOpen(true)
{
    selectFormat();
    initialize();

    // I don't think the following device assignment statements are necessary
    // but in order to avoid the echo pulse noise, I have to do this:
    QList<QAudioDeviceInfo> inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    m_audioInputDevice = inputDevices.at(0);
    QList<QAudioDeviceInfo> outputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    m_audioOutputDevice = outputDevices.at(0);
    QTimer::singleShot(1000, reinterpret_cast<QObject*>(this), SLOT(startPlayback()));
    startRecording();
    // default is shown I guess it is because the value is initialized as defaultInputDevice()

    // used for debug - show the device
    //qDebug() << "Device name: " << m_audioInputDevice.deviceName();
    //qDebug() << "Device name: " << m_audioOutputDevice.deviceName();

    // test first and then make it compatible
//    if (devices.size()>1) {
//        for(int i = 0; i < devices.size(); ++i) {
//            qDebug() << "Device name: " << devices.at(i).deviceName();
//        }
//    }

}

bufferPlayback::~bufferPlayback()
{
    stopRecording();
    stopPlayback();
    delete m_audioInput;
    delete m_audioOutput;
}

// return the audio data length (the length that buffer needs)
//
qint64 bufferPlayback::audioLength(const QAudioFormat &format, qint64 microSeconds)
{
    qint64 result = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
            * microSeconds / 1000000;
    result -= result % (format.channelCount() * format.sampleSize());
    return result;
}

qint64 bufferPlayback::bufferLength() const
{
    return m_bufferLength;
}

void bufferPlayback::initialize()
{
    m_bufferLength = audioLength(format, BufferDurationUs);
    m_buffer.resize(m_bufferLength);
    m_buffer.fill(0);
    m_audioInput = new QAudioInput(m_audioInputDevice, format, this);
    m_audioOutput = new QAudioOutput(m_audioOutputDevice, format, this);
}

void bufferPlayback::startRecording()
{
    if (m_audioInput) {
        m_buffer.fill(0);
        m_mode = QAudio::AudioInput;
        m_dataLengthRecord = 0;
        m_audioInputIODevice = m_audioInput->start();
        connect(m_audioInputIODevice, SIGNAL(readyRead()),
                this,SLOT(captureDataFromDevice()));
    }
}


void bufferPlayback::startPlayback()
{
    if (m_audioOutput) {
        m_mode = QAudio::AudioOutput;
        m_dataLengthPlay = 0;
        m_temp = m_audioOutput->start(/*&m_audioOutputIODevice*/);

        connect (this, SIGNAL(mysignal()),SLOT(captureDataIntoDevice()));
    }
}

void bufferPlayback::stopRecording()
{
    if (m_audioInput) {
        m_audioInput->stop();
        QCoreApplication::instance()->processEvents();
        m_audioInput->disconnect();
    }

    m_audioInputIODevice = 0;
}

void bufferPlayback::stopPlayback()
{
    if (m_audioOutput) {
        m_audioOutput->stop();
        QCoreApplication::instance()->processEvents();
        m_audioOutput->disconnect();
    }
}


void bufferPlayback::selectFormat()
{
    format.setSampleRate(8000);
    format.setChannelCount(2);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo input_info(QAudioDeviceInfo::defaultInputDevice());
    if (!input_info.isFormatSupported(format)) {
        qWarning()<<"default format not supported try to use nearest";
        format = input_info.nearestFormat(format);
    }
    QAudioDeviceInfo output_info(QAudioDeviceInfo::defaultOutputDevice());
    if (!output_info.isFormatSupported(format)) {
        qWarning()<<"raw audio format not supported by backend, cannot play audio.";
    }
}


// push data from buffer into speaker
void bufferPlayback::captureDataIntoDevice()
{
    if (isOpen) {
    m_temp->open(QIODevice::WriteOnly);
    isOpen = false;
    }

    qint64 bytesWrite = m_temp->write(m_buffer.data()+m_dataLengthPlay, m_bytesReady);
    if (bytesWrite) {
        m_dataLengthPlay += bytesWrite;
    }

    if (m_buffer.size() == m_dataLengthPlay) {
        m_dataLengthPlay = 0;
    }
}

// push data from Mic into buffer
void bufferPlayback::captureDataFromDevice()
{
    const qint64 bytesReady = m_audioInput->bytesReady();
    const qint64 bytesSpace = m_buffer.size() - m_dataLengthRecord;  // what is m_dataLength?
    const qint64 bytesToRead = qMin(bytesReady, bytesSpace);
    const qint64 bytesRead = m_audioInputIODevice->read(m_buffer.data()+m_dataLengthRecord,bytesToRead);

    if (bytesRead) {
        m_dataLengthRecord += bytesRead;
    }

    if (m_buffer.size() == m_dataLengthRecord) {
        m_dataLengthRecord = 0;
        //qDebug() << "in capture Data buffer is full";
    }
    emit mysignal();
}

