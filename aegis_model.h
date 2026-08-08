#ifndef AEGIS_MODEL_H
#define AEGIS_MODEL_H

inline int tree_0(int uid, int comm_len, int path_len, int is_root) {
  if (is_root <= 0.50) {
    if (uid <= 1001.50) {
      if (path_len <= 10.00) {
        if (comm_len <= 3.50) {
          return 0;
        } else {
          return 1;
        }
      } else {
        if (path_len <= 12.50) {
          if (path_len <= 11.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        } else {
          if (comm_len <= 3.00) {
            return 1;
          } else {
            if (comm_len <= 4.50) {
              return 0;
            } else {
              return 0;
            }
          }
        }
      }
    } else {
      if (comm_len <= 3.50) {
        return 0;
      } else {
        if (path_len <= 11.00) {
          return 1;
        } else {
          if (path_len <= 13.50) {
            return 0;
          } else {
            return 0;
          }
        }
      }
    }
  } else {
    return 1;
  }
}

inline int tree_1(int uid, int comm_len, int path_len, int is_root) {
  if (is_root <= 0.50) {
    if (path_len <= 10.00) {
      if (comm_len <= 3.50) {
        return 0;
      } else {
        return 1;
      }
    } else {
      if (comm_len <= 3.50) {
        if (uid <= 1001.50) {
          if (comm_len <= 2.50) {
            if (path_len <= 13.50) {
              return 0;
            } else {
              return 1;
            }
          } else {
            return 0;
          }
        } else {
          return 0;
        }
      } else {
        if (uid <= 1001.50) {
          if (comm_len <= 4.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          }
        } else {
          if (path_len <= 13.50) {
            return 0;
          } else {
            return 0;
          }
        }
      }
    }
  } else {
    return 1;
  }
}

inline int tree_2(int uid, int comm_len, int path_len, int is_root) {
  if (uid <= 500.00) {
    return 1;
  } else {
    if (comm_len <= 3.50) {
      if (path_len <= 14.00) {
        if (uid <= 1001.50) {
          if (path_len <= 11.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        } else {
          return 0;
        }
      } else {
        return 1;
      }
    } else {
      if (comm_len <= 4.50) {
        if (path_len <= 11.00) {
          return 1;
        } else {
          if (uid <= 1001.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        }
      } else {
        if (uid <= 1001.50) {
          if (comm_len <= 6.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        } else {
          return 0;
        }
      }
    }
  }
}

inline int tree_3(int uid, int comm_len, int path_len, int is_root) {
  if (path_len <= 17.00) {
    if (uid <= 500.00) {
      return 1;
    } else {
      if (path_len <= 10.00) {
        if (path_len <= 8.50) {
          return 0;
        } else {
          return 1;
        }
      } else {
        if (comm_len <= 3.50) {
          if (path_len <= 14.00) {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 1;
          }
        } else {
          if (comm_len <= 4.50) {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          }
        }
      }
    }
  } else {
    if (uid <= 500.00) {
      return 1;
    } else {
      return 0;
    }
  }
}

inline int tree_4(int uid, int comm_len, int path_len, int is_root) {
  if (is_root <= 0.50) {
    if (path_len <= 10.00) {
      if (comm_len <= 3.50) {
        return 0;
      } else {
        return 1;
      }
    } else {
      if (path_len <= 12.50) {
        if (uid <= 1001.50) {
          if (uid <= 1000.50) {
            if (path_len <= 11.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            if (path_len <= 11.50) {
              return 0;
            } else {
              return 0;
            }
          }
        } else {
          return 0;
        }
      } else {
        if (comm_len <= 3.00) {
          return 1;
        } else {
          if (comm_len <= 4.50) {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          }
        }
      }
    }
  } else {
    return 1;
  }
}

inline int tree_5(int uid, int comm_len, int path_len, int is_root) {
  if (is_root <= 0.50) {
    if (path_len <= 10.00) {
      if (comm_len <= 3.50) {
        return 0;
      } else {
        return 1;
      }
    } else {
      if (path_len <= 12.50) {
        if (path_len <= 11.50) {
          if (uid <= 1001.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        } else {
          return 0;
        }
      } else {
        if (path_len <= 13.50) {
          if (uid <= 1001.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        } else {
          if (comm_len <= 3.50) {
            return 1;
          } else {
            if (comm_len <= 6.50) {
              return 0;
            } else {
              return 0;
            }
          }
        }
      }
    }
  } else {
    return 1;
  }
}

inline int tree_6(int uid, int comm_len, int path_len, int is_root) {
  if (uid <= 500.00) {
    return 1;
  } else {
    if (path_len <= 10.00) {
      if (path_len <= 8.50) {
        return 0;
      } else {
        return 1;
      }
    } else {
      if (path_len <= 12.50) {
        if (path_len <= 11.50) {
          if (uid <= 1001.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        } else {
          return 0;
        }
      } else {
        if (comm_len <= 3.00) {
          return 1;
        } else {
          if (path_len <= 13.50) {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            if (comm_len <= 6.50) {
              return 0;
            } else {
              return 0;
            }
          }
        }
      }
    }
  }
}

inline int tree_7(int uid, int comm_len, int path_len, int is_root) {
  if (uid <= 500.00) {
    return 1;
  } else {
    if (comm_len <= 3.50) {
      if (comm_len <= 2.50) {
        if (uid <= 1001.50) {
          if (path_len <= 13.50) {
            if (uid <= 1000.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 1;
          }
        } else {
          return 0;
        }
      } else {
        return 0;
      }
    } else {
      if (path_len <= 10.50) {
        return 1;
      } else {
        if (uid <= 1001.50) {
          if (path_len <= 13.50) {
            if (comm_len <= 5.00) {
              return 0;
            } else {
              return 0;
            }
          } else {
            if (path_len <= 15.50) {
              return 0;
            } else {
              return 0;
            }
          }
        } else {
          if (path_len <= 13.50) {
            return 0;
          } else {
            return 0;
          }
        }
      }
    }
  }
}

inline int tree_8(int uid, int comm_len, int path_len, int is_root) {
  if (uid <= 500.00) {
    return 1;
  } else {
    if (path_len <= 10.00) {
      if (path_len <= 8.50) {
        return 0;
      } else {
        return 1;
      }
    } else {
      if (comm_len <= 3.50) {
        if (comm_len <= 2.50) {
          if (uid <= 1001.50) {
            if (path_len <= 13.50) {
              return 0;
            } else {
              return 1;
            }
          } else {
            return 0;
          }
        } else {
          return 0;
        }
      } else {
        if (path_len <= 13.50) {
          if (uid <= 1001.50) {
            if (path_len <= 12.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        } else {
          if (comm_len <= 6.50) {
            if (comm_len <= 5.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            return 0;
          }
        }
      }
    }
  }
}

inline int tree_9(int uid, int comm_len, int path_len, int is_root) {
  if (is_root <= 0.50) {
    if (comm_len <= 3.50) {
      if (path_len <= 14.00) {
        if (path_len <= 11.50) {
          if (path_len <= 9.50) {
            return 0;
          } else {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          }
        } else {
          return 0;
        }
      } else {
        return 1;
      }
    } else {
      if (comm_len <= 4.50) {
        if (uid <= 1001.50) {
          if (uid <= 1000.50) {
            if (path_len <= 11.00) {
              return 1;
            } else {
              return 0;
            }
          } else {
            if (path_len <= 11.00) {
              return 1;
            } else {
              return 0;
            }
          }
        } else {
          if (path_len <= 11.00) {
            return 1;
          } else {
            return 0;
          }
        }
      } else {
        if (comm_len <= 6.50) {
          if (comm_len <= 5.50) {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          } else {
            if (uid <= 1001.50) {
              return 0;
            } else {
              return 0;
            }
          }
        } else {
          return 0;
        }
      }
    }
  } else {
    return 1;
  }
}

inline int predict_threat(int uid, int comm_len, int path_len, int is_root) {
    int votes = 0;
    votes += tree_0(uid, comm_len, path_len, is_root);
    votes += tree_1(uid, comm_len, path_len, is_root);
    votes += tree_2(uid, comm_len, path_len, is_root);
    votes += tree_3(uid, comm_len, path_len, is_root);
    votes += tree_4(uid, comm_len, path_len, is_root);
    votes += tree_5(uid, comm_len, path_len, is_root);
    votes += tree_6(uid, comm_len, path_len, is_root);
    votes += tree_7(uid, comm_len, path_len, is_root);
    votes += tree_8(uid, comm_len, path_len, is_root);
    votes += tree_9(uid, comm_len, path_len, is_root);
    return (votes > 5) ? 1 : 0;
}

#endif
