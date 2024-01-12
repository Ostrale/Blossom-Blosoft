# Implementation d'une structure de gestion de données avec C++

Dans le cadre de notre projet visant à lutter contre la pollution numérique, nous avons choisi d'utiliser la puissante structure de données `unordered_map` en C++ pour stocker efficacement nos informations. Explorons comment cette structure fonctionne et comment nous l'appliquons à notre projet.

## Utilisation d'`unordered_map` en C++

L'`unordered_map` est une structure de données associant des clés à des valeurs de manière efficace. Elle est basée sur le concept de table de hachage, ce qui permet un accès rapide aux éléments. Dans notre cas, nous l'utiliserons pour associer des informations de flux (`flow_info`) à des structures de flux (`flow_struct`).

```cpp
#include <unordered_map>

struct flow_info{
    int family; // AF_INET ou AF_INET6
        uint32_t src_ip; // Make union if possible
        Tins::IPv6::address_type src_ipv6;
        uint32_t dst_ip;
        Tins::IPv6::address_type dst_ipv6;
    uint16_t src_port;
    uint16_t dst_port;
    ndpi_protocol detected_protocol; 
    bool is_tcp;
};

struct flow_struct {
    flow_info info;
    ndpi_flow_struct ndpi_flow;
    std::vector<packet_struct> packets;
};

std::unordered_map<flow_info, std::unique_ptr<flow_struct>> dataMap;
```

## Rapidité de unordered_map
L'`unordered_map` offre des performances rapides en moyenne pour les opérations d'insertion, de recherche et de suppression. Grâce à son implémentation basée sur une table de hachage, l'accès aux éléments se fait en temps constant en moyenne, ce qui est crucial pour notre applicatio

## Gestion des timestamps avec une "queue" C++

Nous avons également besoin de rechercher rapidement les données par timestamp. Pour ce faire, nous créerons une file (`queue`) en C++, naturellement ordonnée par timestamp.

```cpp
#include <queue>

struct TimestampQueue {
    // Définir la structure TimestampQueue
};

std::queue<TimestampQueue> timestampQueue;
```

Chaque élément de la file sera associé à un pointeur vers notre objet dans `unordered_map`, permettant ainsi de supprimer rapidement toutes les données jusqu'au timestamp souhaité.

## Gain de temps vs Surcoût en mémoire
L'utilisation d'une `unordered_map` offre une rapidité significative pour les opérations de recherche, mais il est important de noter qu'elle peut entraîner un surcoût en mémoire à cause de la `queue` qui duplique de l'information. Nous devons évaluer attentivement ce surcoût en fonction des besoins spécifiques de notre application, en tenant compte des avantages en termes de performance.

## Intégration des structures

Maintenant, voyons comment intégrer ces structures dans notre logiciel.

```cpp
// Ajout d'un élément à unordered_map
flow_info info;
std::unique_ptr<flow_struct> flowData = std::make_unique<flow_struct>();
dataMap[info] = std::move(flowData);

// Ajout d'un élément à la file avec timestamp
TimestampQueue timestampData;
timestampData.timestamp = /* définir le timestamp */;
timestampData.pointer = &dataMap[info];
timestampQueue.push(timestampData);

// Suppression des données jusqu'au timestamp spécifié
while (!timestampQueue.empty() && timestampQueue.front().timestamp <= /* timestamp à atteindre */) {
    dataMap.erase(*timestampQueue.front().pointer);
    timestampQueue.pop();
}
```

Ainsi, notre structure de gestion de données permet d'associer des informations de flux à des structures de flux tout en facilitant la recherche et la suppression basées sur les timestamps. Cela devrait contribuer efficacement à la réalisation de notre projet de lutte contre la pollution numérique.
