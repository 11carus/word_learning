#pragma once

#include <QAbstractScrollArea>
#include <QImage>
#include <QPdfSelection>
#include <QRectF>

#include <optional>

class QPdfDocument;
class QMouseEvent;
class QWheelEvent;

class PdfSelectableView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit PdfSelectableView(QWidget *parent = nullptr);

    void setDocument(QPdfDocument *document);
    void setCurrentPage(int page);
    int currentPage() const;
    void setZoomFactor(qreal factor);
    qreal zoomFactor() const;
    void fitToWidth();
    QString selectedText() const;

signals:
    void currentPageChanged(int page);
    void zoomFactorChanged(qreal factor, bool fitsWidth);
    void textSelected(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateRenderedPage();
    QPointF pagePointAt(const QPointF &position) const;
    QPointF viewPointAt(const QPointF &position) const;
    void updateSelection(const QPointF &position);

    QPdfDocument *m_document = nullptr;
    int m_currentPage = 0;
    QImage m_pageImage;
    QSize m_renderedSize;
    QSize m_logicalPageSize;
    QRectF m_pageRect;
    QPointF m_dragStart;
    std::optional<QPdfSelection> m_selection;
    bool m_dragging = false;
    qreal m_zoomFactor = 1.0;
    bool m_fitsWidth = true;
};
