template<typename T>
    requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
std::vector<ArithmeticOperation<T>> evaluate_ops(
    ArithmeticOperation<T>::Level at_level,
    std::vector<ArithmeticOperation<T>>& ops
)
{
    std::vector<ArithmeticOperation<T>> evaluated;

    std::function<T(const ArithmeticOperation<T>&)> handle_second_level =
        [&](const ArithmeticOperation<T>& second_level) -> T
    {
        T second_level_val = 0;

        /* Iterate first over all values to see if multiplication occurs. */
        std::vector<ArithmeticOperation<T>> cleaned;

        for (size_t i = 0; i < second_level.second_level_ops.size(); i++)
        {
            if (second_level.second_level_ops.at(i)->operation == ArithmeticOperation<T>::MULT)
            {
                if (i + 1 < second_level.second_level_ops.size() &&
                    second_level.second_level_ops.at(i + 1)->level == ArithmeticOperation<T>::SECOND)
                {
                    ArithmeticOperation<T> next =
                        std::move(*second_level.second_level_ops.at(i + 1).get());

                    T internal_second_level_val = handle_second_level(next);

                    cleaned.push_back(std::move(*second_level.second_level_ops.at(i).get()));
                    cleaned.push_back(ArithmeticOperation<T>{
                        .level     = ArithmeticOperation<T>::FIRST, // collapse the second level
                        .value     = internal_second_level_val,
                        .operation = next.operation
                    });

                    i++;
                }
                else
                {
                    cleaned.push_back(std::move(*second_level.second_level_ops.at(i).get()));
                }
            }
            else
            {
                if (i + 1 < second_level.second_level_ops.size() &&
                    second_level.second_level_ops.at(i + 1)->level == ArithmeticOperation<T>::SECOND)
                {
                    ArithmeticOperation<T> next =
                        std::move(*second_level.second_level_ops.at(i + 1).get());

                    T internal_second_level_val = handle_second_level(next);

                    cleaned.push_back(std::move(*second_level.second_level_ops.at(i).get()));
                    cleaned.push_back(ArithmeticOperation<T>{
                        .level     = ArithmeticOperation<T>::FIRST, // collapse the second level
                        .value     = internal_second_level_val,
                        .operation = next.operation
                    });
                }
                else
                {
                    cleaned.push_back(std::move(*second_level.second_level_ops.at(i).get()));
                }
            }
        }

        std::vector<ArithmeticOperation<T>> cleaned2;
        for (size_t i = 0; i < cleaned.size(); i++)
        {
            if (cleaned.at(i).operation == ArithmeticOperation<T>::MULT)
            {
                ArithmeticOperation<T> current = std::move(cleaned.at(i));

                if (current.level == ArithmeticOperation<T>::SECOND)
                    current.value = handle_second_level(current);

                if (cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                {
                    i++;

                    if (cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                    {
                        T div_val = static_cast<T>(0);
                        int round = 0;

                        while (i + 1 < cleaned.size() &&
                               cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                        {
                            if (round != 0)
                                div_val = div_val / cleaned.at(i + 1).value;
                            else
                            {
                                div_val = cleaned.at(i).value / cleaned.at(i + 1).value;
                                round++;
                            }

                            i++;
                        }

                        cleaned2.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = current.value * (div_val / cleaned.at(i + 1).value),
                            .operation = cleaned.at(i + 1).operation
                        });

                        cleaned.at(i + 1).value = cleaned2.back().value;

                        i++;
                        continue;
                    }
                    else
                    {
                        T div_val = cleaned.at(i).value / cleaned.at(i + 1).value;

                        cleaned2.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = current.value * div_val,
                            .operation = cleaned.at(i + 1).operation
                        });

                        cleaned.at(i + 1).value = cleaned2.back().value;

                        i++;
                        continue;
                    }
                }

                cleaned2.push_back(ArithmeticOperation<T>{
                    .level     = ArithmeticOperation<T>::FIRST,
                    .value     = current.value * cleaned.at(i + 1).value,
                    .operation = cleaned.at(i + 1).operation
                });

                cleaned.at(i + 1).value = cleaned2.back().value;

                i++;
                continue;
            }
            else if (cleaned.at(i).operation == ArithmeticOperation<T>::DIV)
            {
                ArithmeticOperation<T> current = std::move(cleaned.at(i));

                if (current.level == ArithmeticOperation<T>::SECOND)
                    current.value = handle_second_level(current);

                if (cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                {
                    T div_val = static_cast<T>(0);
                    int round = 0;

                    while (i + 1 < cleaned.size() &&
                           cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                    {
                        if (round != 0)
                        {
                            ArithmeticOperation<T> internal = std::move(cleaned.at(i));

                            if (internal.level == ArithmeticOperation<T>::SECOND)
                                internal.value = handle_second_level(current);

                            div_val = div_val / internal.value;
                        }
                        else
                        {
                            div_val = current.value / cleaned.at(i + 1).value;
                            round++;
                        }

                        i++;
                    }

                    cleaned2.push_back(ArithmeticOperation<T>{
                        .level     = ArithmeticOperation<T>::FIRST,
                        .value     = div_val / ops.at(i + 1).value,
                        .operation = cleaned.at(i + 1).operation
                    });

                    cleaned.at(i + 1).value = cleaned2.back().value;

                    i++;
                    continue;
                }
                else
                {
                    if (cleaned.at(i + 1).operation == ArithmeticOperation<T>::MULT)
                    {
                        T div_val = current.value / cleaned.at(i + 1).value;

                        i++;
                        cleaned2.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = div_val * cleaned.at(i + 1).value,
                            .operation = cleaned.at(i + 1).operation
                        });
                        cleaned.at(i + 1).value = cleaned2.back().value;
                    }
                    else
                    {
                        cleaned2.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = current.value / cleaned.at(i + 1).value,
                            .operation = cleaned.at(i + 1).operation
                        });

                        cleaned.at(i + 1).value = cleaned2.back().value;
                    }
                }

                i++;
                continue;
            }

            cleaned2.push_back(std::move(cleaned.at(i)));
        }

        cleaned = std::move(cleaned2);

        for (size_t i = 0; i < cleaned.size(); i++)
        {
            if (cleaned.at(i).level == ArithmeticOperation<T>::SECOND)
            {
                switch (cleaned.at(i).operation)
                {
                    case ArithmeticOperation<T>::ADD:
                    {
                        if (i + 1 >= cleaned.size())
                        {
                            second_level_val += handle_second_level(std::move(cleaned.at(i)));
                        }
                        else
                        {
                            second_level_val +=
                                handle_second_level(std::move(cleaned.at(i))) + cleaned.at(i + 1).value;
                            i++;
                            cleaned.at(i).value = second_level_val;
                        }
                        break;
                    }
                    case ArithmeticOperation<T>::SUB:
                    {
                        if (i + 1 >= cleaned.size())
                        {
                            second_level_val -= handle_second_level(std::move(cleaned.at(i)));
                        }
                        else
                        {
                            second_level_val +=
                                handle_second_level(std::move(cleaned.at(i))) - cleaned.at(i + 1).value;
                            i++;
                            cleaned.at(i).value = second_level_val;
                        }
                        break;
                    }
                    case ArithmeticOperation<T>::DIV:
                    {
                        if (i + 1 >= cleaned.size())
                        {
                            second_level_val /= handle_second_level(std::move(cleaned.at(i)));
                        }
                        else
                        {
                            second_level_val +=
                                handle_second_level(std::move(cleaned.at(i))) / cleaned.at(i + 1).value;
                            i++;
                            cleaned.at(i).value = second_level_val;
                        }
                        break;
                    }
                    case ArithmeticOperation<T>::MULT:
                    {
                        if (i + 1 >= cleaned.size())
                        {
                            second_level_val *= handle_second_level(std::move(cleaned.at(i)));
                        }
                        else
                        {
                            second_level_val =
                                handle_second_level(std::move(cleaned.at(i))) * cleaned.at(i + 1).value;
                            i++;
                            cleaned.at(i).value = second_level_val;
                        }
                        break;
                    }
                    case ArithmeticOperation<T>::NONE:
                    {
                        break;
                    }
                }
                continue;
            }

            switch (cleaned.at(i).operation)
            {
                case ArithmeticOperation<T>::ADD:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        second_level_val = cleaned.back().value + cleaned.at(i).value;
                    }
                    else
                    {
                        if (cleaned.at(i + 1).level == ArithmeticOperation<T>::SECOND)
                        {
                            ArithmeticOperation<T> next = std::move(cleaned.at(i + 1));
                            second_level_val = cleaned.at(i).value + handle_second_level(next);
                            cleaned.at(i + 1) = std::move(next);

                            i++;
                            cleaned.at(i).value = second_level_val;
                        }
                        else
                        {
                            second_level_val = cleaned.at(i).value + cleaned.at(i + 1).value;
                            cleaned.at(i + 1).value = second_level_val;
                        }
                    }
                    break;
                }
                case ArithmeticOperation<T>::SUB:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        second_level_val = cleaned.back().value - cleaned.at(i).value;
                    }
                    else
                    {
                        if (cleaned.at(i + 1).level == ArithmeticOperation<T>::SECOND)
                        {
                            ArithmeticOperation<T> next = std::move(cleaned.at(i + 1));
                            second_level_val = cleaned.at(i).value - handle_second_level(next);
                            cleaned.at(i + 1) = std::move(next);

                            i++;
                            cleaned.at(i).value = second_level_val;
                        }
                        else
                        {
                            second_level_val = cleaned.at(i).value - cleaned.at(i + 1).value;
                            cleaned.at(i + 1).value = second_level_val;
                        }
                    }
                    break;
                }
                case ArithmeticOperation<T>::DIV:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        second_level_val = cleaned.back().value / cleaned.at(i).value;
                    }
                    else
                    {
                        if (cleaned.at(i + 1).level == ArithmeticOperation<T>::SECOND)
                        {
                            ArithmeticOperation<T> next = std::move(cleaned.at(i + 1));
                            second_level_val = cleaned.at(i).value / handle_second_level(next);
                            cleaned.at(i + 1) = std::move(next);

                            i++;
                            cleaned.at(i).value = second_level_val;
                        }
                        else
                        {
                            second_level_val = cleaned.at(i).value / cleaned.at(i + 1).value;
                            cleaned.at(i + 1).value = second_level_val;
                        }
                    }
                    break;
                }
                case ArithmeticOperation<T>::MULT:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        second_level_val = cleaned.back().value * cleaned.at(i).value;
                    }
                    else
                    {
                        if (cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                        {
                            ArithmeticOperation<T> current = std::move(cleaned.at(i));
                            i++;

                            if (cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                            {
                                T div_val = static_cast<T>(0);
                                int round = 0;

                                while (i + 1 < cleaned.size() &&
                                       cleaned.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                                {
                                    if (round != 0)
                                        div_val = div_val / cleaned.at(i + 1).value;
                                    else
                                    {
                                        div_val = cleaned.at(i).value / cleaned.at(i + 1).value;
                                        round++;
                                    }

                                    i++;
                                }

                                second_level_val = current.value * (div_val / cleaned.at(i + 1).value);
                                cleaned.at(i + 1).value = second_level_val;

                                i++;
                                continue;
                            }
                            else
                            {
                                T div_val = cleaned.at(i).value / cleaned.at(i + 1).value;
                                second_level_val = current.value * div_val;
                                cleaned.at(i + 1).value = second_level_val;

                                i++;
                                continue;
                            }
                        }
                        else
                        {
                            second_level_val = cleaned.at(i).value * cleaned.at(i + 1).value;
                            cleaned.at(i + 1).value = second_level_val;
                        }
                    }
                    break;
                }
                case ArithmeticOperation<T>::NONE:
                {
                    second_level_val = cleaned.at(i).value;
                    break;
                }
            }
        }

    done:
        return second_level_val;
    };

    if (at_level == ArithmeticOperation<T>::SECOND)
    {
        /* This "collapses" any second level, or nested second level, to a first level. */
        for (size_t i = 0; i < ops.size(); i++)
        {
            if (ops.at(i).level == ArithmeticOperation<T>::SECOND)
            {
                T handled = handle_second_level(ops.at(i));
                evaluated.push_back(ArithmeticOperation<T>{
                    .level     = ArithmeticOperation<T>::FIRST,
                    .value     = handled,
                    .operation = ops.at(i).operation
                });
            }
            else
            {
                evaluated.push_back(std::move(ops.at(i)));
            }
        }
    }
    else
    {
        /* Iterate first over all values to see if multiplication occurs. */
        std::vector<ArithmeticOperation<T>> cleaned;

    redo:
        for (size_t i = 0; i < ops.size(); i++)
        {
            if (ops.at(i).operation == ArithmeticOperation<T>::MULT)
            {
                if (ops.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                {
                    ArithmeticOperation<T> current = std::move(ops.at(i));
                    i++;

                    if (ops.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                    {
                        T div_val = static_cast<T>(0);
                        int round = 0;

                        while (i + 1 < ops.size() &&
                               ops.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                        {
                            if (round != 0)
                                div_val = div_val / ops.at(i + 1).value;
                            else
                            {
                                div_val = ops.at(i).value / ops.at(i + 1).value;
                                round++;
                            }

                            i++;
                        }

                        cleaned.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = current.value * (div_val / ops.at(i + 1).value),
                            .operation = ops.at(i + 1).operation
                        });

                        ops.at(i + 1).value = cleaned.back().value;

                        i++;
                        continue;
                    }
                    else
                    {
                        T div_val = ops.at(i).value / ops.at(i + 1).value;

                        cleaned.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = current.value * div_val,
                            .operation = ops.at(i + 1).operation
                        });

                        ops.at(i + 1).value = cleaned.back().value;

                        i++;
                        continue;
                    }
                }

                cleaned.push_back(ArithmeticOperation<T>{
                    .level     = ArithmeticOperation<T>::FIRST,
                    .value     = ops.at(i).value * ops.at(i + 1).value,
                    .operation = ops.at(i + 1).operation
                });

                ops.at(i + 1).value = cleaned.back().value;

                i++;
                continue;
            }
            else if (ops.at(i).operation == ArithmeticOperation<T>::DIV)
            {
                if (ops.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                {
                    T div_val = static_cast<T>(0);
                    int round = 0;

                    while (i + 1 < ops.size() &&
                           ops.at(i + 1).operation == ArithmeticOperation<T>::DIV)
                    {
                        if (round != 0)
                            div_val = div_val / ops.at(i + 1).value;
                        else
                        {
                            div_val = ops.at(i).value / ops.at(i + 1).value;
                            round++;
                        }

                        i++;
                    }

                    cleaned.push_back(ArithmeticOperation<T>{
                        .level     = ArithmeticOperation<T>::FIRST,
                        .value     = div_val / ops.at(i + 1).value,
                        .operation = ops.at(i + 1).operation
                    });

                    ops.at(i + 1).value = cleaned.back().value;

                    i++;
                    continue;
                }
                else
                {
                    if (ops.at(i + 1).operation == ArithmeticOperation<T>::MULT)
                    {
                        T div_val = ops.at(i).value / ops.at(i + 1).value;

                        i++;
                        cleaned.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = div_val * ops.at(i + 1).value,
                            .operation = ops.at(i + 1).operation
                        });
                        ops.at(i + 1).value = cleaned.back().value;
                    }
                    else
                    {
                        cleaned.push_back(ArithmeticOperation<T>{
                            .level     = ArithmeticOperation<T>::FIRST,
                            .value     = ops.at(i).value / ops.at(i + 1).value,
                            .operation = ops.at(i + 1).operation
                        });

                        ops.at(i + 1).value = cleaned.back().value;
                    }
                }

                i++;
                continue;
            }

            cleaned.push_back(std::move(ops.at(i)));
        }

        for (const auto& m : cleaned)
        {
            std::cout << m.value
                      << (m.operation == ArithmeticOperation<T>::MULT
                              ? " * "
                              : m.operation == ArithmeticOperation<T>::DIV
                                    ? " / "
                                    : m.operation == ArithmeticOperation<T>::SUB
                                          ? " - "
                                          : " + ");
        }

        for (const auto& m : cleaned)
        {
            if (m.operation == ArithmeticOperation<T>::MULT)
            {
                ops = std::move(cleaned);
                cleaned.clear();
                goto redo;
            }
        }

        T evaluated_value = 0;

        for (size_t i = 0; i < cleaned.size(); i++)
        {
            switch (cleaned.at(i).operation)
            {
                case ArithmeticOperation<T>::MULT:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        evaluated_value *= cleaned.at(i).value;
                    }
                    else
                    {
                        evaluated_value = cleaned.at(i).value * cleaned.at(i + 1).value;
                        cleaned.at(i + 1).value = evaluated_value;
                    }
                    break;
                }
                case ArithmeticOperation<T>::ADD:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        evaluated_value += cleaned.at(i).value;
                    }
                    else
                    {
                        evaluated_value = cleaned.at(i).value + cleaned.at(i + 1).value;
                        cleaned.at(i + 1).value = evaluated_value;
                    }
                    break;
                }
                case ArithmeticOperation<T>::SUB:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        evaluated_value -= cleaned.at(i).value;
                    }
                    else
                    {
                        evaluated_value = cleaned.at(i).value - cleaned.at(i + 1).value;
                        cleaned.at(i + 1).value = evaluated_value;
                    }
                    break;
                }
                case ArithmeticOperation<T>::DIV:
                {
                    if (i + 1 >= cleaned.size())
                    {
                        evaluated_value /= cleaned.at(i).value;
                    }
                    else
                    {
                        evaluated_value = cleaned.at(i).value / cleaned.at(i + 1).value;
                        cleaned.at(i + 1).value = evaluated_value;
                    }
                    break;
                }
                case ArithmeticOperation<T>::NONE:
                {
                    if (i == 0)
                        evaluated_value += cleaned.at(i).value;
                    break;
                }
            }
        }

        evaluated.push_back(ArithmeticOperation<T>{
            .level     = ArithmeticOperation<T>::FIRST,
            .value     = evaluated_value,
            .operation = ArithmeticOperation<T>::NONE
        });
    }

    return evaluated;
}

template<typename T>
    requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
std::vector<ArithmeticOperation<T>> process_raw_arithmetics(
    TeaConfig& program_config,
    Lexer* lexer,
    TeaToken*& tokens,
    size_t& _token_idx
)
{
    const auto consume_token = [&](bool just_try = false, bool error_if_last_token = true)
    {
        if (!just_try)
        {
            TEA_ERR_IF(_token_idx >= tokens->tokens,
                       "OOPS",
                       lexer != nullptr ? lexer->get_line() : this->line)
        }

        if (_token_idx >= tokens->tokens)
        {
            return TeaToken{ .is_last = true };
            tokens = lexer != nullptr ? lexer->get_tokens() : this->get_tokens();
            _token_idx = 0;
        }

        TEA_ERR_IF_V2(error_if_last_token && tokens->is_last,
                      "Unexpected end of file",
                      program_config.get_filename().data(),
                      tokens->line,
                      "ensure your code is complete and does not end abruptly")

        _token_idx++;
        return tokens[_token_idx - 1];
    };

    std::vector<ArithmeticOperation<T>> raw_operation;
    TeaToken token;

    /* Store the first value. */
    while (!(token = consume_token(true)).is_last)
    {
        if (token.token == R_PAREN)
            goto ret;

        if (token.token == L_PAREN)
        {
            /* Already starting with a "second level". */
            std::vector<ArithmeticOperation<T>> layer =
                this->process_raw_arithmetics<T>(program_config, lexer, tokens, _token_idx);

            token = consume_token(true);

            std::vector<std::unique_ptr<ArithmeticOperation<T>>> slo;

            for (auto& l_v : layer)
                slo.push_back(std::make_unique<ArithmeticOperation<T>>(std::move(l_v)));

            raw_operation.push_back(ArithmeticOperation<T>{
                .level = ArithmeticOperation<T>::SECOND,
                .operation =
                    token.is_last || token.token == R_PAREN
                        ? ArithmeticOperation<T>::NONE
                        : token.token == PLUS
                              ? ArithmeticOperation<T>::ADD
                              : token.token == ASTERISK
                                    ? ArithmeticOperation<T>::MULT
                                    : token.token == FORWARD_SLASH
                                          ? ArithmeticOperation<T>::DIV
                                          : ArithmeticOperation<T>::SUB,
                .second_level_ops = std::move(slo)
            });

            if (token.token == R_PAREN || token.is_last)
                goto ret;

            continue;
        }

        T val = token.token == T_DECIMAL
                    ? static_cast<T>(std::stof(token.token_value))
                    : static_cast<T>(std::stoi(token.token_value,
                                               nullptr,
                                               token.token == T_HEX ? 16 : 10));

        token = consume_token(true);

        raw_operation.push_back(ArithmeticOperation<T>{
            .level = ArithmeticOperation<T>::FIRST,
            .value = val,
            .operation =
                token.is_last || token.token == R_PAREN // `is_last` or `)` indicates the "end"
                    ? ArithmeticOperation<T>::NONE
                    : val < 0
                          ? ArithmeticOperation<T>::SUB
                          : token.token == PLUS
                                ? ArithmeticOperation<T>::ADD
                                : token.token == ASTERISK
                                      ? ArithmeticOperation<T>::MULT
                                      : token.token == FORWARD_SLASH
                                            ? ArithmeticOperation<T>::DIV
                                            : ArithmeticOperation<T>::SUB
        });

        if (token.token == R_PAREN || token.is_last)
            goto ret;
    }

ret:
    return raw_operation;
}

template<typename T>
    requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
T handle_arithmetic(
    TeaConfig& program_config,
    Lexer* lexer,
    TeaToken*& tokens,
    size_t& _token_idx
)
{
    std::vector<ArithmeticOperation<T>> raw_operation =
        this->process_raw_arithmetics<T>(program_config, lexer, tokens, _token_idx);

    for (const auto& ro : raw_operation)
    {
        std::cout << "level: "
                  << (ro.level == ArithmeticOperation<T>::SECOND ? "second" : "first");

        if (ro.level == ArithmeticOperation<T>::SECOND)
        {
            std::cout << "\n\tSecond Level Ops: " << ro.second_level_ops.size() << std::endl;

            for (const auto& sro : ro.second_level_ops)
            {
                std::cout << "\tlevel: "
                          << (sro->level == ArithmeticOperation<T>::SECOND ? "second" : "first");

                if (sro->level == ArithmeticOperation<T>::SECOND)
                {
                    std::cout << "\n\t\tSecond Level Ops: " << sro->second_level_ops.size()
                              << std::endl;
                }
                else
                {
                    std::cout << "\n\t\tValue: " << sro->value << std::endl;
                }

                std::cout << "\tOperation: "
                          << (sro->operation == ArithmeticOperation<T>::ADD
                                  ? "ADD"
                                  : sro->operation == ArithmeticOperation<T>::SUB
                                        ? "SUB"
                                        : sro->operation == ArithmeticOperation<T>::MULT
                                              ? "MULT"
                                              : "END")
                          << std::endl;
            }
        }
        else
        {
            std::cout << "\n\tValue: " << ro.value << std::endl;
        }

        std::cout << "Operation: "
                  << (ro.operation == ArithmeticOperation<T>::ADD
                          ? "ADD"
                          : ro.operation == ArithmeticOperation<T>::SUB
                                ? "SUB"
                                : ro.operation == ArithmeticOperation<T>::MULT
                                      ? "MULT"
                                      : ro.operation == ArithmeticOperation<T>::DIV
                                            ? "DIV"
                                            : "END")
                  << std::endl;
    }

    std::cout << "okay!\n";

    std::vector<ArithmeticOperation<T>> evaluated =
        this->evaluate_ops<T>(ArithmeticOperation<T>::SECOND, raw_operation);

    evaluated = this->evaluate_ops<T>(ArithmeticOperation<T>::FIRST, evaluated);

    return evaluated.at(0).value;
}
