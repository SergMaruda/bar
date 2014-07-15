#pragma once

namespace TransactionStatus
  {
  enum ETrStatus
    {
    WaitingForPayment = 0,
    Payed = 1,
    WaitingForPurchasePayment = 2,
    PurchasePayed = 3,
    Canceled = 4,
    Goodwithdrawal = 5,
    GoodPayedCostPrice = 6,
    WrittenOff = 7,
    NotUsed = 8
    };
  };
