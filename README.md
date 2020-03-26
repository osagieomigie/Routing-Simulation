<h1 align="center">Welcome to Routing-Simulation 👋</h1>
<p>
</p>

> This project explores the effectiveness of different routing algorithms for a circuit-switched network. It essentially evaluates the relative performance of these algorithms with respect to several performance metrics when handling circuit-oriented Skype calls. 

> The specific routing algorithms that the program models :

    - Shortest Hop Path First (SHPF): This algorithm tries to find the shortest path currently available from source to destination, where the length of a path refers to the number of hops (i.e., links) traversed. 

    - Shortest Delay Path First (SDPF): This algorithm tries to find the shortest path currently available from source to destination, where the length of a path refers to the cumulative total propagation delay for traversing the chosen links in the path. 

    - Least Loaded Path (LLP): This algorithm tries to find the least loaded path currently available from source to destination, where the load of a path is determined by the "busiest" link on the path, and the load on a link is its current utilization (i.e., the ratio of its current number of active calls to the capacity C of that link for carrying calls). 

    - Maximum Free Circuits (MFC): This algorithm tries to find the currently available path from source to destination that has the most free circuits, where the number of free circuits for a candidate path is the smallest number of free circuits currently observed on any link in that possible path. 

## Install 
```sh
compile main with: g++ -o main main.cpp     
```

## Usage
```sh
    ./main
```

## Test

```sh
This software was tested with network topology of 16 nodes (with 23 bidirectional links), and call work load of 10000 calls.
```

## Author

👤 **Osagie Omigie**

* Website: osagie.me

## Show your support

Give a ⭐️ if this project helped you!

***
_This README was generated with ❤️ by [readme-md-generator](https://github.com/kefranabg/readme-md-generator)_