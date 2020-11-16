
#include <QApplication>
#include <QImage>
#include <QDebug>
#include <QFile>
#include <QPixmap>
#include <QRgb>
#include <fstream>
#include <map>
#include <vector>
#include <QDir>
void colorTest(const QString& path, const QString& name)
{
    QImage im(path);
    QImage img(im.width(), im.height(), im.format());
    if (im.hasAlphaChannel())
        qDebug() << "Yes";
    for (int j = 0; j < im.height(); ++j)
    {
        for (int i = 0; i < im.width(); ++i)
        {
            QRgb color = im.pixel(i, j);
            int res1, res2, res3 = 0;
            if (qRed(color) & 0x1)
                res1 = 255;
            else
                res1 = 0;
            if (qGreen(color) & 0x1)
                res2 = 255;
            else
                res2 = 0;
            if (qBlue(color) & 0x1)
                res3 = 255;
            else
                res3 = 0;

            QRgb newColor = qRgb(res1, res2, res3);
            img.setPixel(i, j, newColor);
        }
    }
    img.save(name);
}

QRgb setSymbol(const QRgb& color, unsigned char code)
{
    int red = qRed(color);
    int green = qGreen(color);
    int blue = qBlue(color);
    int time = 0;
    time = red & 0xFC;
    red = time + code & 0x3;
    time = green & 0xF8;
    green = time + ((code >> 2) & 0x7);
    time = blue & 0xF8;
    blue = time + ((code >> 5) & 0x7);
    return qRgb(red, green, blue);
}

unsigned char getSymbol(const QRgb& color)
{
    unsigned char code = 0;
    unsigned int red = qRed(color);
    code |= (red & 0x3);

    unsigned int green = qGreen(color);
    code |= (green & 0x7) << 2;

    unsigned int blue = qBlue(color);
    code |= (blue & 0x7) << 5;
    return code;
}


bool encoder(QImage& img, const QString& string, const QString& path)
{

    QFile file(QString(PROJECT_PATH) + "/text.txt");
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error";
        return false;
    }
    QTextStream in(&file);
    qint64 len = file.size();
    QString sizeText = QString::number(len);
    QString text;
    if (!string.isEmpty())
    {
        len = string.length();
        sizeText = QString::number(len);
        text = string;
    }
    else
    {
        text = in.readAll();
    }
    int size = 0;
    int i = 0, j = 0;
    unsigned char code = '#'; //0x23
    QRgb color = img.pixel(i, j);
    QRgb newColor = setSymbol(color, code);
    img.setPixel(i++, j, newColor);

    for (int k = 0; k < sizeText.length(); ++k)
    {
        if (j >= img.height())
            return false;
        if (i >= img.width())
        {
             i = 0;
             ++j;
        }
        newColor = setSymbol(color, sizeText[k].cell());
        img.setPixel(i++, j, newColor);

    }
    newColor = setSymbol(color, code);
    img.setPixel(i++, j, newColor);

    while (size < len)
    {
        if (j == img.height())
            break;
        if (i == img.width())
        {
             i = 0;
             ++j;
        }
        color = img.pixel(i, j);
        newColor = setSymbol(color, text[size].cell());
        img.setPixel(i++, j, newColor);
        ++size;
    }
    img.save(path);

    return true;
}

bool decoder(const QImage& img, QString& str)
{
    int i = 0, j = 0;
    QString sizeText;
    QRgb color = img.pixel(i++, j);

    unsigned char time = getSymbol(color);
    if (time != '#')
        return false;
    if (j == img.height())
        return false;
    if (i == img.width())
    {
         i = 0;
         ++j;
    }
    color = img.pixel(i++, j);
    time = getSymbol(color);
    while (time != '#')
    {
        if (j == img.height())
            break;
        if (i == img.width())
        {
             i = 0;
             ++j;
        }
        sizeText.push_back(time);
        color = img.pixel(i++, j);
        time = getSymbol(color);
    }
    int length = sizeText.toInt();
    QString text(length);
    for (int k = 0; k < length; ++k)
    {
        if (j >= img.height())
            return false;
        if (i >= img.width())
        {
             i = 0;
             ++j;
        }
        color = img.pixel(i++, j);
        text[k] = getSymbol(color);
    }
    str = text;
    return true;
}

bool chi_square(QImage& image)
{
    int count = 0;

    for (int j = 0; j < image.height(); ++j)
    {
        std::vector<unsigned int> expected;
        std::vector<unsigned int> observed;
        std::vector<unsigned int> colorVector(256);
        for (int i = 0; i < image.width(); ++i)
            colorVector[qRed(image.pixel(i, j))] += 1;
        for (int k = 0; k <= 127; ++k)
        {
            expected.push_back(((colorVector[2 * k] + colorVector[2 * k + 1]) / 2));
            observed.push_back(colorVector[2 * k]);

        }
        unsigned int Sum = 0;
        for (int k = 0; k <= 127; ++k)
        {
            if (expected[k] == 0)
                continue;
            Sum += (((observed[k] - expected[k])*(observed[k] - expected[k])) / expected[k]);
        }

        if (Sum < 5)
        {
            for (int i = 0; i < image.width(); ++i)
                image.setPixel(i, j, qRgb(255, 0, 0));
            ++count;
        }
    }
    image.save(QString(PROJECT_PATH) + "/ChiSquare.png");
    qDebug() << ((float)count / image.height()) * 100 << "%";
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString str;
    colorTest(QString(PROJECT_PATH) + "/image1.png", QString(PROJECT_PATH) + "/imageColorBeforeEncoder.png");
    QImage image(QString(PROJECT_PATH) + "/image1.png");
    encoder(image, "", QString(PROJECT_PATH) + "/imageAfterEncoder.png");
    decoder(image, str);
    qDebug() << str;
    colorTest(QString(PROJECT_PATH) + "/imageAfterEncoder.png", QString(PROJECT_PATH) + "/imageColorAfterEncoder.png");
    QImage im(QString(PROJECT_PATH) + "/imageAfterEncoder.png");
    chi_square(im);
    return 0;
}
