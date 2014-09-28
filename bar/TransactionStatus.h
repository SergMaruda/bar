#pragma once

namespace TransactionStatus
  {
  enum ETrStatus
    {
    WaitingForPayment = 0,
    Sold = 1,
    WaitingForPurchasePayment = 2,
    Bought = 3,
    PrepareGoodwithdrawal = 5,
    Goodwithdrawal = 6,
    WrittenOff = 7,
    SaleCanceled = 8,
    PaydedWithDiscount = 9,
    Unknown = 999,
    };
  };
