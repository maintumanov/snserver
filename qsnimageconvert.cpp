#include "qsnimageconvert.h"


QString qsnAvatarBase64(QString filename, int width, int height)
{
    if (QFile::exists(filename)) {
        QImage img(width, height, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&img);
        //----------------------
        QPen pen;
        pen.setStyle(Qt::NoPen);
        painter.setPen(pen);

        QImage photo(filename);
        // Указываем изображение в качестве паттерна

        photo = photo.scaled(QSize(img.width(), img.height()),Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QBrush brush(photo);

        painter.setBrush(brush);

        // Рисуем прямоугольник с закруглёнными краями
        qreal radius = 2;
        painter.drawRoundedRect(QRectF(0, 0, img.width(), img.height()), radius, radius);
        painter.end();

        QByteArray raw;
        QBuffer buffer(&raw);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");
        buffer.close();
        return QString("data:image/png;base64,%1").arg(QString(raw.toBase64()));
    }
    return QString();
}

QString qsnImageToBase64(QString filename)
{
    QFile img(filename);
    if (img.open(QIODevice::ReadOnly)) {
        QByteArray raw = img.readAll();
        img.close();
        return QString(raw.toBase64());
    }
    return QString();
}

QString qsnHtmlBase64(QString data, QString format)
{
    if (data.isEmpty()) return QString();
    return QString("data:image/%1;base64,%2").arg(format).arg(data);
}

QString qsnHtmlSVG(QString filename)
{
    QString svg;
    QFile file(filename);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return svg;
        QTextStream streamread(&file);
        svg =  streamread.readAll();
        file.close();
        int i = svg.indexOf("<svg");
        svg.remove(0, i);
        i = svg.indexOf("</svg>");
        svg.remove(i + 6, svg.count() - i - 6);
        return QString("data:image/svg+xml;utf8,%1").arg(svg);
    }
    return svg;
}

void qsnBase64SavePNG(QString data, QString filename)
{
    QString img = data;
    if (img.indexOf("data:image/png;base64,") != 0) return;
    img.remove(0, 22);
    QByteArray raw;
    raw = QByteArray::fromBase64(img.toUtf8());
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(raw);
    file.close();
}

void qsnBase64SaveResizePNG(QString data, QString filename)
{
    QString img = data;
    QImage image;
        if (img.indexOf("data:image/png;base64,") == 0) {
            img.remove(0, 22);
            image = QImage::fromData(QByteArray::fromBase64(img.toUtf8()), "PNG");
        }  else if (img.indexOf("data:image/jpeg;base64,") == 0) {
            img.remove(0, 23);
            image = QImage::fromData(QByteArray::fromBase64(img.toUtf8()), "JPG");
        } else return;

    image = image.scaled(QSize(256,256),Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image.save(filename, "PNG");
}
