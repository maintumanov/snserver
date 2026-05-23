#ifndef QSNIMAGECONVERT_H
#define QSNIMAGECONVERT_H
#include <QObject>
#include <QImage>
#include <QPainter>
#include <QFile>
#include <QBuffer>
#include <QTextStream>
#include <QDebug>
#include <QImageReader>

QString qsnAvatarBase64(QString filename, int width, int height);
QString qsnImageToBase64(QString filename);
QString qsnHtmlBase64(QString data, QString format);
QString qsnHtmlSVG(QString filename);
void qsnBase64SavePNG(QString data, QString filename);
void qsnBase64SaveResizePNG(QString data, QString filename);

#endif
