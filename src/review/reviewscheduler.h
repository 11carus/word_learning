#pragma once

#include <QDate>

enum class ReviewRating
{
    Again = 1,
    Hard = 2,
    Good = 3,
    Easy = 4
};

class ReviewScheduler
{
public:
    static QDate nextReviewDate(ReviewRating rating, const QDate &today = QDate::currentDate());
};

