#pragma once
#include <QValidator>

class QTextEncodingValidator : public QValidator {
   Q_OBJECT
   public:
      QTextEncodingValidator(QObject* parent = nullptr);
      //
      QByteArray encoding() const noexcept;
      void setEncoding(const QByteArray& name);
      QTextCodec* textCodec() const noexcept;
      void setTextCodec(QTextCodec*);
      //
      inline bool strict() const noexcept { return this->_strict; } // controls whether bad chars are Invalid or Intermediate
      void setStrict(bool);
      //
      virtual QValidator::State validate(QString& input, int& pos) const override;
      //
   signals:
      void strictnessChanged(bool);
      void textCodecChanged(QTextCodec*);
      //
   protected:
      QTextCodec* _codec  = nullptr;
      bool        _strict = false;
};