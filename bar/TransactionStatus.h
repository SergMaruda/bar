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
    PrepareGoodwithdrawal = 5,
    Goodwithdrawal = 6,
    WrittenOff = 7,
    SaleCanceled = 8,
    PaydedWithDiscount = 9,
    Unknown = 999,
    };
  };
