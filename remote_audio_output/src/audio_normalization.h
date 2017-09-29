#ifndef AUDIO_NORMALIZATION_H
#define AUDIO_NORMALIZATION_H

#include <QWidget>

class AudioNormalization : public QObject
{
    Q_OBJECT

public:
    explicit AudioNormalization(QObject *parent=0);

    void proc(QByteArray *data, int channels);

    void setUpdateTime(uint16_t ms);
    void setGainChangeStep(double value);
    void setMaximumLevelPercentage(double value);

private:
    qint64 last_update_timestamp;
    qint64 update_time;

    double gain_factor;
    double gain_change_step;
    double maximum_level_percentage;

    int32_t max_level;

signals:
    void gainFactorChanged(double value);
};

#endif // AUDIO_NORMALIZATION_H
