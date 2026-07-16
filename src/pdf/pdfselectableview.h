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

    QPdfDocument *m_document = nullptr;              // 当前显示的 PDF 文档，不拥有其生命周期。
    int m_currentPage = 0;                           // 当前页码，使用从 0 开始的索引。
    QImage m_pageImage;                              // 按当前尺寸渲染并缓存的页面图像。
    QSize m_renderedSize;                            // 页面图像的物理像素尺寸，包含高 DPI 倍率。
    QSize m_logicalPageSize;                         // 页面在界面坐标系中的逻辑显示尺寸。
    QRectF m_pageRect;                               // 页面在视口中的位置和逻辑尺寸。
    QPointF m_dragStart;                             // 鼠标按下位置对应的 PDF 页面坐标。
    std::optional<QPdfSelection> m_selection;        // 当前文字选区及其页面坐标边界。
    bool m_dragging = false;                         // 是否正在按住鼠标左键拖选文字。
    qreal m_zoomFactor = 1.0;                        // 相对于适宽比例的用户缩放倍数。
    bool m_fitsWidth = true;                         // 当前是否处于适合窗口宽度模式。
};
