#include "review/reviewscheduler.h"

QDate ReviewScheduler::nextReviewDate(ReviewRating rating, const QDate &today)
{
    switch (rating) {
    case ReviewRating::Again:
        return today.addDays(1);
    case ReviewRating::Hard:
        return today.addDays(2);
    case ReviewRating::Good:
        return today.addDays(4);
    case ReviewRating::Easy:
        return today.addDays(7);
    }

    return today.addDays(1);
}

