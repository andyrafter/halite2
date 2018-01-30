import math

def min_dist_squared( px, py, ux, uy, qx, qy, vx, vy ):
    # p is 1st point 
    # u is distance moved by p in 1 time step
    # q is 2nd point
    # v is distance moved by q in 1 time step

    # interested in points at time t {0..1}:
    #
    # P = p + u * t
    # Q = q + v * t
    #
    # vector separating points
    #
    # D = P - Q = p-q + (u-v)* t
    #
    # Let pq = p - q and uv = u - v
    #
    # D = pq + uv * t
    #
    # D.x = pq.x + uv.x * t
    # D.y = pq.y + uv.y * t
    #
    # dist_squared = D**2 = (pq.x + uv.x * t)**2 + (pq.y + uv.y * t)**2
    #                     = pq.x**2 + 2*pq.x*uv.x*t + uv.x**2 * t**2 + 
    #                       pq.y**2 + 2*pq.y*uv.y*t + uv.y**2 * t**2
    #                     = (uv.x**2 + uv.y**2)       * t**2 +
    #                       2*(pq.x*uv.x + pq.y*uv.y) * t + 
    #                       (pq.x**2 + pq.y**2)
    #
    # let A = uv.x**2 + uv.y**2
    # let B = 2*(pq.x*uv.x + pq.y*uv.y)
    # let C = pq.x**2 + pq.y**2
    #
    # D**2 = A * t**2 + B * t + C
    #
    # derivative: d(D**2)/dt = 2*A * t + B
    # is 0 at minimum.
    #
    # 2*A * t + B == 0
    # t = -B / (2 * A)
    #
    # if t is -ve, then cannot hit if not already.
    # if t is > 1, then use distance at t == 1.

    pqx = px - qx
    pqy = py - qy
    uvx = ux - vx
    uvy = uy - vy

    print(pqx, pqy, uvx, uvy)

    A = uvx * uvx + uvy * uvy
    print(A)
    B = 2 * (pqx * uvx + pqy * uvy)
    print(B)
    C = pqx * pqx + pqy * pqy
    print(C)

    if A == 0:
        # velocity vectors are parallel and have same magnitude.
        # distance apart is constant, so use distance at t == 0.
        return C


    # limit t to be no further away than 1 time step into future.
    t = min(1.0, -B / (2 * A))

    # for -ve t return dist at t == 0.
    if t < 0:
        return C

    # work out distance apart at time t.
    return t * t * A + t * B + C

dist = math.sqrt(min_dist_squared(0, 0, 7, 0,  0, 2, 6, -1))
print(dist)

dist = math.sqrt(min_dist_squared(0, 0, 1, 1,  2, 0, -1, 1))
print(dist)
