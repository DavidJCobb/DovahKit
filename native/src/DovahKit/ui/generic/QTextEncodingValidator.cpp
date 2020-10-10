#include "QTextEncodingValidator.h"
#include <QTextCodec>

QTextEncodingValidator::QTextEncodingValidator(QObject* parent) : QValidator(parent) {
}

QByteArray QTextEncodingValidator::encoding() const noexcept {
   if (this->_codec)
      return this->_codec->name();
   return QByteArray();
}
QTextCodec* QTextEncodingValidator::textCodec() const noexcept {
   return this->_codec;
}

void QTextEncodingValidator::setEncoding(const QByteArray& name) {
   this->_codec = QTextCodec::codecForName(name);
   emit changed();
   emit textCodecChanged(this->_codec);
}
void QTextEncodingValidator::setTextCodec(QTextCodec* codec) {
   this->_codec = codec;
   emit changed();
   emit textCodecChanged(this->_codec);
}

QValidator::State QTextEncodingValidator::validate(QString& input, int& pos) const {
   if (!this->_codec || input.isEmpty())
      return QValidator::State::Acceptable;
   if (this->_codec->canEncode(input))
      return QValidator::State::Acceptable;
   if (this->_strict)
      return QValidator::State::Invalid;
   return QValidator::State::Intermediate;
}

void QTextEncodingValidator::setStrict(bool s) {
   this->_strict = true;
   emit changed();
   emit strictnessChanged(this->_strict);
}