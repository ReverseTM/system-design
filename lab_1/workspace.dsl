workspace "Архитектура сервиса доставки посылок" {

    model {
        guest = person "Гость" "Незарегистрированный пользователь"
        user = person "Пользователь" "Зарегистрированный пользователь"
        admin = person "Администратор" "Администратор системы"

        paymentSystem = softwareSystem "Платежная система" "Обработка платежей за услуги доставки" "External"
        notificationSystem = softwareSystem "Сервис уведомлений" "Отправка SMS и email уведомлений о статусе доставки" "External"

        deliverySystem = softwareSystem "Сервис доставки" "Платформа для управления посылками и доставками между пользователями" {

            webApp = container "Web Application" "Веб-интерфейс для пользователей: регистрация, вход, создание посылок, оформление доставки, отслеживание" "React, TypeScript" "Web Browser"

            apiGateway = container "API Gateway" "Единая точка входа: маршрутизация запросов к микросервисам, валидация JWT" "Nginx"

            authService = container "Auth Service" "Аутентификация и авторизация пользователей: регистрация, вход, выдача и валидация JWT-токенов" "C++, userver"

            userService = container "User Service" "Управление пользователями: регистрация, поиск по логину, поиск по маске имя/фамилия" "C++, userver"

            packageService = container "Package Service" "Управление посылками: создание посылки, получение списка посылок пользователя" "C++, userver"

            deliveryService = container "Delivery Service" "Управление доставками: создание доставки, получение информации по отправителю и получателю" "C++, userver"

            notificationService = container "Notification Service" "Формирование и отправка уведомлений пользователям о статусе доставки" "C++, userver"

            paymentConfirmedTopic = container "payment.confirmed" "Топик подтверждения оплаты от Payment Service к Delivery Service" "Kafka Topic" "Queue"

            deliveryCreatedTopic = container "delivery.created" "Топик создания доставки от Delivery Service к Notification Service" "Kafka Topic" "Queue"

            deliveryStatusChangedTopic = container "delivery.status_changed" "Топик изменения статуса доставки от Delivery Service к Notification Service" "Kafka Topic" "Queue"

            userDB = container "User Database" "Хранение данных пользователей: профили, логины, контактная информация" "PostgreSQL" "Database"

            packageDB = container "Package Database" "Хранение данных о посылках: вес, размеры, описание, владелец" "PostgreSQL" "Database"

            deliveryDB = container "Delivery Database" "Хранение данных о доставках: маршрут, статус, отправитель, получатель" "PostgreSQL" "Database"

            paymentService = container "Payment Service" "Управление платежами: инициация оплаты, обработка вебхуков, история транзакций" "C++, userver"

            paymentDB = container "Payment Database" "Хранение транзакций, статусов платежей и истории операций" "PostgreSQL" "Database"

            redisCache = container "Cache" "Кэширование часто запрашиваемых данных для снижения нагрузки на БД" "Redis" "Cache"            
        }

        guest -> deliverySystem "Просматривает информацию о сервисе, регистрируется"
        user -> deliverySystem "Создаёт посылки, оформляет доставку, отслеживает отправления и получает посылки"
        admin -> deliverySystem "Управляет системой и пользователями"

        guest -> webApp "Просматривает сервис и регистрируется" "HTTPS/REST"
        user -> webApp "Создаёт посылки, оформляет доставку, отслеживает" "HTTPS/REST"
        admin -> webApp "Управляет системой" "HTTPS/REST"

        deliverySystem -> notificationSystem "Отправляет SMS/email уведомления"
        deliverySystem -> paymentSystem "Проводит оплату"

        webApp -> apiGateway "Отправляет все запросы" "HTTPS/REST"

        apiGateway -> authService "Валидирует JWT, маршрутизирует запросы аутентификации" "HTTPS/REST"
        apiGateway -> userService "Маршрутизирует запросы управления пользователями" "HTTPS/REST"
        apiGateway -> packageService "Маршрутизирует запросы управления посылками" "HTTPS/REST"
        apiGateway -> deliveryService "Маршрутизирует запросы управления доставками" "HTTPS/REST"

        packageService -> packageDB "Читает и записывает данные посылок" "JDBC"
        packageService -> userService "Проверяет существование пользователя-владельца" "HTTPS/REST"

        deliveryService -> deliveryDB "Читает и записывает данные доставок" "JDBC"
        deliveryService -> redisCache "Кэширует и читает данные доставок" "Redis Protocol"
        deliveryService -> userService "Проверяет существование отправителя и получателя" "HTTPS/REST"
        deliveryService -> packageService "Получает информацию о посылке" "HTTPS/REST"
        deliveryService -> paymentService "Инициирует оплату доставки" "HTTPS/REST"
        deliveryService -> deliveryCreatedTopic "Записывает событие о создании доставки" "Kafka"
        deliveryService -> deliveryStatusChangedTopic "Записывает событие об изменении статуса доставки" "Kafka"
        deliveryService -> paymentConfirmedTopic "Читает событие" "Kafka"

        paymentService -> paymentDB "Читает и записывает транзакции и статусы" "JDBC"
        paymentService -> paymentConfirmedTopic "Записывает событие" "Kafka"
        paymentService -> paymentSystem "Инициирует оплату" "HTTP/REST"

        notificationService -> deliveryCreatedTopic "Читает событие о создании доставки" "Kafka"
        notificationService -> deliveryStatusChangedTopic "Читает событие об изменении статуса доставки" "Kafka"
        notificationService -> notificationSystem "Отправляет SMS/email уведомления" "HTTPS/REST"

        authService -> userDB "Проверяет учётные данные, хранит сессии" "JDBC"

        userService -> userDB "Читает и записывает данные пользователей" "JDBC"
    }

    views {
        systemContext deliverySystem "SystemContext" {
            include *
            autoLayout
            title "C1: System Context — Сервис доставки"
        }

        container deliverySystem "Containers" {
            include *
            autoLayout
            title "C2: Container — Сервис доставки"
        }

        dynamic deliverySystem "CreateDelivery" "Создание доставки от пользователя к пользователю" {
            user -> webApp "Заполняет форму доставки (посылка, получатель, адрес)"
            webApp -> apiGateway "POST /deliveries"
            apiGateway -> authService "Валидация JWT-токена"
            authService -> apiGateway "Токен валиден"
            apiGateway -> deliveryService "Маршрутизирует запрос на создание доставки"
            deliveryService -> userService "GET /users/{senderId} и GET /users/{recipientId} — проверка пользователей"
            userService -> deliveryService "Данные отправителя и получателя подтверждены"
            deliveryService -> packageService "GET /packages/{packageId} — получение данных о посылке"
            packageService -> deliveryService "Данные посылки получены"
            deliveryService -> paymentService "POST /payments — инициирует оплату"
            paymentService -> paymentSystem "Перенаправляет запрос в платёжный шлюз"
            paymentSystem -> paymentService "Вебхук: платёж подтверждён"
            paymentService -> paymentConfirmedTopic "Записывает payment.confirmed"
            deliveryService -> paymentConfirmedTopic "Читает payment.confirmed"
            deliveryService -> deliveryDB "INSERT delivery — сохранение данных доставки"
            deliveryService -> deliveryCreatedTopic "Записывает событие delivery.created"
            notificationService -> deliveryCreatedTopic "Читает событие delivery.created"
            notificationService -> notificationSystem "Отправка SMS/email получателю о новой посылке"
            deliveryService -> apiGateway "Возвращает данные созданной доставки"
            apiGateway -> webApp "Ответ с данными доставки"
            webApp -> user "Отображает информацию о созданной доставке и трек-номер"
            autoLayout tb
            title "Dynamic: Создание доставки от пользователя к пользователю"
        }

        styles {
            element "Person" {
                shape Person
                background #08427b
                color #ffffff
                fontSize 14
            }
            element "Software System" {
                background #1168bd
                color #ffffff
            }
            element "External" {
                background #999999
                color #ffffff
            }
            element "Container" {
                background #438dd5
                color #ffffff
            }
            element "Database" {
                shape Cylinder
                background #438dd5
                color #ffffff
            }
            element "Web Browser" {
                shape WebBrowser
                background #438dd5
                color #ffffff
            }
            element "Queue" {
                shape Pipe
                background #438dd5
                color #ffffff
            }
            element "Cache" {
                shape Cylinder
                background #e8735a
                color #ffffff
            }
        }
    }
}
